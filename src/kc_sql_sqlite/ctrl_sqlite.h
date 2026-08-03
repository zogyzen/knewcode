#pragma once

#include "std.h"

class CCtrlSqlite : public IKCController
{
public:
    CCtrlSqlite(IKCSqlite&, string, property_tree::ptree&);
    ~CCtrlSqlite() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

protected:
    // 获取类型名
    static CSqlFunc::EDataType GetTypeName(unsigned int oid);
    // 获取数据库文件的绝对路径
    string GetDBFile(ICtrlApiData& objCtrlD);

protected:
    // sqlite 异常类
    struct TSqliteException : public std::runtime_error
    {
        int m_code = 0;
        string m_err;
        string m_msg;
        string m_pos;

        TSqliteException(int c, string e, string s, string p);

        // 检查错误
        template<int VOK = SQLITE_OK>
        static void CheckError(sqlite3 &connDB, int resStatus, string sLogInfo, string pos)
        {
            if(VOK != resStatus)
            {
                string sSttStr = algorithm::trim_copy(CUtilFunc::PCharSafeToStr(sqlite3_errstr(resStatus)));
                string sErrMsg = algorithm::trim_copy(CUtilFunc::PCharSafeToStr(sqlite3_errmsg(&connDB)));
                string sMsg;
                if (sErrMsg.empty())
                {
                    sErrMsg = sSttStr;
                    sMsg = "\n" + sSttStr + "\r\n" + sLogInfo;
                }
                else sMsg = "\n" + sSttStr + "\n" + sErrMsg + "\n[ToUtf8::   " + CUtilFunc::GbkToUtf8(sErrMsg) + "].\r\n" + sLogInfo;
                // if (!sPgErr.empty()) sPgErr = CUtilFunc::Utf8ToGbk(sPgErr);
                throw TSqliteException(resStatus, sErrMsg, sMsg, pos);
                //throw TSqliteException(resStatus, lexical_cast<string>(PQresultStatus(res)) + "\n" + sPgErr);
            }
        }
    };

    // 保持数据库执行实例
    struct KeepStmtSqlite
    {
        sqlite3_stmt& m_stmt;
        KeepStmtSqlite(sqlite3_stmt &s) : m_stmt(s) {}
        ~KeepStmtSqlite(void) { sqlite3_finalize(&m_stmt); }
    };
    typedef std::shared_ptr<KeepStmtSqlite> KeepStmtSqlitePtr;

    // 参数
    struct TParmGrpSqlite;
    struct TParmSqlite : public CSqlFunc::TParm
    {
        TParmGrpSqlite &m_own;
        string sDbTP = "varchar";
        string sOther;

        TParmSqlite(TParmGrpSqlite& own, std::string = "", unsigned = 0);

        // 设置参数值
        bool SetNumber(double) override;
        void SetInt(int) override;
        void SetString(string) override;
        void SetClob(string) override;
        void SetNull(void) override;
        // 设置参数值的函数是否有效
        bool IsValidSetXX(void) const override { return false; }
        // 获取参数值
        string GetVal(string = "");
        string GetString(string = "") override;
        string GetClob(string = "") override;
        string GetDate(string = "%Y-%m-%d") override;
        string GetDateTime(string = "%Y-%m-%d %H:%M:%S") override;
        int GetInt(int = 0) override;
        double GetNumber(double = 0) override;
        // 获取游标数据集
        CSqlFunc::TRecordSetPtr GetDbSet(void) override;

        void SetDbTp(void) override;

        void RegOutParam(void) override;
    };
    typedef CSqlFunc::TParmGrp<TSqliteException> TParmGrpSqliteBase;
    struct TParmGrpSqlite : public TParmGrpSqliteBase
    {
        CCtrlSqlite& m_own;
        ICtrlApiData& m_objCtrlD;
        string m_kcFile, m_actName;
        sqlite3 &m_con;

        TParmGrpSqlite(CCtrlSqlite& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, sqlite3 &con);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // 创建参数
        CSqlFunc::TParmPtr MakePram(std::string = "", unsigned = 0) override;

        // 异常信息
        string ParmExceptInfo(TSqliteException&) override;

        // 是否检查遗漏参数（都检查）
        bool IsCheckLeakParms() override {return true; }
    };

    struct TDBCommandSqlite;
    // 数据集
    struct TRecordSetSqlite : public CSqlFunc::TRecordSet
    {
        TDBCommandSqlite& m_own;
        ICtrlApiData& m_objCtrlD;
        KeepStmtSqlitePtr m_keepDbSet;
        sqlite3_stmt& m_dbset;
        const int m_dataCount = 0, m_fieldCount = 0;

        TRecordSetSqlite(TDBCommandSqlite& own, ICtrlApiData& ctrlD, KeepStmtSqlitePtr);
        ~TRecordSetSqlite(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        // 获取字段信息
        void GetColumnListMetaData(CSqlFunc::TFields &vecFeilds) override;
        // 下一条
        bool Next(void) override;
        // 添加值
        void AddVal(IKCJson& jsonRow, CSqlFunc::TField&) override;
        // 得到值
        CSqlFunc::TKcValVariant GetVal(CSqlFunc::TField&) const override;
    };
    friend struct TRecordSetSqlite;

    // 数据库执行命令
    struct TDBCommandSqlite : public CSqlFunc::TDBCommand<TSqliteException>
    {
        CCtrlSqlite& m_own;
        ICtrlApiData& m_objCtrlD;
        const string m_kcFile, m_actName, m_stmtName, m_method;
        sqlite3 &m_con;
        KeepStmtSqlitePtr m_keepStmt;
        sqlite3_stmt *m_stmt = nullptr;
        string m_sql;
        string m_tranSavePnt;
        TParmGrpSqlite m_parms;
        unsigned m_resultParmCount = 0;

        TDBCommandSqlite(CCtrlSqlite& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, sqlite3 &con, string sSQL, string sMethod);
        ~TDBCommandSqlite(void);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // SQL语句
        std::string GetSQL(void) override { return m_sql; }

        // 参数组
        TParmGrpSqliteBase& GetParms(void) override { return m_parms; }

        // 异常信息
        std::tuple<int, std::string, std::string> ParmExceptInfo(TSqliteException& ex) override
        {
            return make_tuple(ex.m_code, ex.m_msg, ex.m_pos);
        }

        // 事务
        void TranBegin(void) override;
        void TranCommit(void) override;
        void TranRollback(void) override;

        // 预备sql语句
        void PrepareSQL(void) override;

        // 批量操作
        pair<int, string> ExecuteBatch(void) override;

        // 执行
        unsigned int ExecuteUpdate(void) override;

        // 查询
        CSqlFunc::TRecordSetPtr ExecuteQuery(int &rows_affected) override;

        // 执行过程
        int ExecuteProcedures(void) override;

        // 仿postgresql的copy
        void CopyBatch(void);

    protected:
        // 绑定参数
        string bindParm(void);
        // 执行
        int UpdateOrQuery(string&);
        unsigned int ExecuteUpdate(bool);
        int ExecuteProcedures(bool);

        // 预执行DDL语句
        void PrepareByDDLSQL(void);
        // 预执行单语句
        void PrepareByOneSQL(void);
    };
    friend struct TDBCommandPgSQL;

    // 页面活动对象（用于保存数据库连接，在整个页面执行期间有效）
    struct TSqliteConnObj final : public IActionData::TActObj
    {
        // 数据库连接
        sqlite3 &m_con;

        TSqliteConnObj(CCtrlSqlite& own, ICtrlApiData& objCtrlD);
        void Release(void) override;
    };
    friend struct TSqliteConnObj;

private:
    // 宿主
    IKCSqlite& m_own;
    // 数据库连接
    const string m_name = "connect1", m_dbFile = "./db", m_PRAGMA = "PRAGMA journal_mode = WAL;";
};
