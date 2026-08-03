#pragma once

#include "std.h"

class CCtrlPgSQL : public IKCController
{
public:
    CCtrlPgSQL(IKCSqlPgSQL&, string, property_tree::ptree&);
    ~CCtrlPgSQL() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

protected:
    // 获取类型名
    static CSqlFunc::EDataType GetTypeName(unsigned int oid);
    static unsigned int GetOIDByTp(string);

protected:
    // libpq异常类
    struct TLibPQException : public std::runtime_error
    {
        int m_code = 0;
        string m_err;
        string m_msg;
        string m_pos;

        TLibPQException(int c, string e, string s, string p);

        // 检查错误
        static ExecStatusType CheckError(PGresult* res, string pos);
        template<ExecStatusType VOK = PGRES_COMMAND_OK>
        static void CheckError(PGresult* res, string sLogInfo, string pos)
        {
            auto resStatus = TLibPQException::CheckError(res, pos);
            if(VOK != resStatus)
            {
                string sSttErr = (boost::format("<%s> ExecStatusType Error: %s != %s") % PQcmdStatus(res) % PQresStatus(resStatus) % PQresStatus(VOK)).str();
                string sPgErr = algorithm::trim_copy(CUtilFunc::PCharSafeToStr(PQresultErrorMessage(res)));
                string sMsg;
                if (sPgErr.empty())
                {
                    sPgErr = sSttErr;
                    sMsg = "\n" + sSttErr + "\r\n" + sLogInfo;
                }
                else sMsg = "\n" + sSttErr + "\n" + sPgErr + "\n[ToUtf8::   " + CUtilFunc::GbkToUtf8(sPgErr) + "].\r\n" + sLogInfo;
                // if (!sPgErr.empty()) sPgErr = CUtilFunc::Utf8ToGbk(sPgErr);
                throw TLibPQException(resStatus, sPgErr, sMsg, pos);
                //throw TLibPQException(resStatus, lexical_cast<string>(PQresultStatus(res)) + "\n" + sPgErr);
            }
        }
    };

    // 参数
    struct TParmGrpPgSQL;
    struct TParmPgSQL : public CSqlFunc::TParm
    {
        TParmGrpPgSQL &m_own;
        string sDbTP = "varchar";
        unsigned int m_oid = /*VARCHAROID*/0;   // 让引擎自动推导
        string sAliasDyn = sName;   // 动态别名（主要针对临时表）

        TParmPgSQL(TParmGrpPgSQL& own, string = "", unsigned = 0);

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
        // 获取临时表数据集
        CSqlFunc::TRecordSetPtr GetDbSet(void) override;
        // 是否临时表
        bool IsTempTable(void) const override { return CSqlFunc::EDataType::edtDbSet == eDTp || CSqlFunc::EDataType::edtDbSetOne == eDTp || CSqlFunc::EDataType::edtDbSetArray == eDTp || CSqlFunc::EDataType::edtDbSetInner == eDTp; }

        void SetDbTp(void) override;
    };
    typedef CSqlFunc::TParmGrp<TLibPQException> TParmGrpPgSQLBase;
    struct TParmGrpPgSQL : public TParmGrpPgSQLBase
    {
        CCtrlPgSQL& m_own;
        ICtrlApiData& m_objCtrlD;
        string m_kcFile, m_actName;
        PGconn &m_con;

        TParmGrpPgSQL(CCtrlPgSQL& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, PGconn &con);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // 创建参数
        CSqlFunc::TParmPtr MakePram(std::string = "", unsigned = 0) override;

        // 异常信息
        string ParmExceptInfo(TLibPQException&) override;

        // 是否检查遗漏参数（都检查）
        bool IsCheckLeakParms() override {return true; }
    };
    friend struct TParmGrpPgSQL;

    // 数据集
    struct TRecordSetPgSQL : public CSqlFunc::TRecordSet
    {
        CCtrlPgSQL& m_own;
        ICtrlApiData& m_objCtrlD;
        PGresult& m_dbset;
        const int m_recCount = 0, m_fieldCount = 0;
        int m_resStep = -1;

        TRecordSetPgSQL(CCtrlPgSQL& own, ICtrlApiData& ctrlD, PGresult&);
        ~TRecordSetPgSQL(void) override;

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
    friend struct TRecordSetPgSQL;

    // 数据库执行命令
    struct TDBCommandPgSQL : public CSqlFunc::TDBCommand<TLibPQException>
    {
        CCtrlPgSQL& m_own;
        ICtrlApiData& m_objCtrlD;
        const string m_kcFile, m_actName, m_stmtName, m_method;
        PGconn &m_con;
        string m_sql, m_funcCreateSQL, m_execSQL;
        string m_tranSavePnt;
        TParmGrpPgSQL m_parms;
        unsigned m_resultParmCount = 0;
        bool m_batchIsProc = false;

        TDBCommandPgSQL(CCtrlPgSQL& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, PGconn &con, string sSQL, string sMethod);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // SQL语句
        std::string GetSQL(void) override { return m_sql; }

        // 参数组
        TParmGrpPgSQLBase& GetParms(void) override { return m_parms; }

        // 异常信息
        std::tuple<int, std::string, std::string> ParmExceptInfo(TLibPQException& ex) override
        {
            return make_tuple(ex.m_code, ex.m_msg, ex.m_pos);
        }

        // 事务
        void TranBegin(void) override;
        void TranCommit(void) override;
        void TranRollback(void) override;
        void TranSavePoint(string) override;
        void TranRollbackToSavePoint(void) override;

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

        // 调用临时函数
        void PerformFunc(void);

        // postgresql特有的批量数据导入
        void CopyFromStdinByCsv(void);

    protected:
        // 执行
        PGresult* UpdateOrQuery(string&);
        unsigned int ExecuteUpdate(bool);
        int ExecuteProcedures(bool);

        // 预执行DDL语句
        void PrepareByDDLSQL(void);
        // 预执行过程
        void PrepareByProcedures(void);
        // 预执行单语句
        void PrepareByOneSQL(void);
        // 预执行保存点SQL
        void PrepareBySavePoint(void);
    };
    friend struct TDBCommandPgSQL;

    // 页面活动对象（用于保存数据库连接，在整个页面执行期间有效）
    struct TPgConnObj final : public IActionData::TActObj
    {
        PGconn *m_conn = nullptr;

        TPgConnObj(CCtrlPgSQL& own);
        void Release(void) override;
    };
    friend struct TPgConnObj;

private:
    // 宿主
    IKCSqlPgSQL& m_own;
    // 名称
    string m_name = "connect1";
    // 连接串
    string m_connStr = "hostaddr=127.0.0.1 dbname=test user=postgres password=1";
    // 连接串是否加密
    bool m_encryptConnStr = false;
    // 参数名称区分大小写
    bool m_parmCaseSensitive = false;
    // 过程返回的临时表数据集，临时表名称是否动态可变，默认不变（针对金仓数据库，建议设置为可变）
    bool m_dynTmpTableName = false;
    // 动态库的被加载类型
    string m_loadType;
};
