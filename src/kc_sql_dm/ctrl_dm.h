#pragma once

#include "std.h"

class CCtrlDM : public IKCController
{
public:
    CCtrlDM(IKCSqlDM&, string, property_tree::ptree&);
    ~CCtrlDM() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

protected:
    // 获取类型名
    static CSqlFunc::EDataType GetTypeName(int tp);
    // 得到编码
    string GetCharset(void) const;

protected:
    // 达梦DPI异常类
    struct TDmdpiException : std::runtime_error
    {
        int m_code = 0;
        string m_msg;
        string m_pos;

        TDmdpiException(int c, string s, string p);

        // 检查错误
        static void CheckError(CCtrlDM& own, DPIRETURN rt, sdint2 hndl_type, dhandle hndl, string sPos, string sKey = "");
    };
    friend struct TDmdpiException;

    // 页面活动对象（用于保存数据库连接，在整个页面执行期间有效）
    struct TDmConnObj final : public IActionData::TActObj
    {
        CCtrlDM& m_own;
        dhcon m_conn = nullptr;
        const unsigned m_connid = 0;

        TDmConnObj(CCtrlDM& own, unsigned connid);
        void Release(void) override;
    };
    friend struct TDmConnObj;

    // 保持数据库执行实例
    struct KeepStmtDM
    {
        dhstmt &stmt;
        KeepStmtDM(dhstmt &s) : stmt(s) {}
        ~KeepStmtDM(void) { try { dpi_free_stmt(stmt); } catch (...) {} }
    };
    typedef std::shared_ptr<KeepStmtDM> KeepStmtDMPtr;

    // 参数
    struct TParmGrpDM;
    struct TParmDM : public CSqlFunc::TParm
    {
        TParmGrpDM &m_own;
        // 参数值
        union
        {
            int i = 0;
            double d;
        } m_nVal;
        vector<char> m_sVal;
        slength c_ind = 0;

        // dm参数信息
        sdint2         sql_type     = DSQL_VARCHAR;
        ulength        prec         = 0;
        sdint2         scale        = 0;
        sdint2         nullable     = 0;

        TParmDM(TParmGrpDM&, std::string = "", unsigned = 0);

        // 设置参数值
        bool SetNumber(double) override;
        void SetInt(int) override;
        void SetString(string) override;
        void SetClob(string) override;
        void SetNull(void) override;
        // 获取参数值
        bool IsNull(void) override;
        string GetString(string = "") override;
        string GetClob(string = "") override;
        string GetDate(string = "yyyy-MM-dd") override;
        string GetDateTime(string = "yyyy-MM-dd hh:mm:ss") override;
        int GetInt(int = 0) override;
        double GetNumber(double = 0) override;
        // 获取游标数据集
        CSqlFunc::TRecordSetPtr GetDbSet(void) override;

        void SetDbTp(void) override;

        void RegOutParam(void) override;

        // 得到dm参数信息
        void GetParmDesc(void);
    };
    typedef CSqlFunc::TParmGrp<TDmdpiException> TParmGrpDMBase;
    struct TParmGrpDM : public TParmGrpDMBase
    {
        CCtrlDM& m_own;
        dhcon &conn;
        KeepStmtDMPtr keepStmt;
        dhstmt &stmt;
        ICtrlApiData& m_objCtrlD;
        string m_act;
        // 参数
        std::map<std::string, CSqlFunc::TParmPtr, TLessStr> mapDmParms;
        // 达梦的参数描述
        struct TDmParmDesc
        {
            sdint2         sql_type;
            ulength        prec;
            sdint2         scale;
            sdint2         nullable;
        };
        std::vector<TDmParmDesc> m_dmParmDesc;

        TParmGrpDM(CCtrlDM& ctrl, dhcon& cn, KeepStmtDMPtr st, ICtrlApiData& re, string sAct);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // 创建参数
        CSqlFunc::TParmPtr MakePram(std::string = "", unsigned = 0) override;

        // 在数据库上解绑所有已绑定的参数
        void UnbindParms(void) override;

        // 异常信息
        string ParmExceptInfo(TDmdpiException&) override;
    };

    // 字段
    struct TRecordSetDM;
    struct TFieldDM : public CSqlFunc::TField
    {
        // 字段值
        union
        {
            int i = 0;
            double d;
        } m_nVal;
        vector<char> m_sVal;
        slength c_ind = 0;

        TFieldDM(TRecordSetDM&, std::string n, int p, int t, unsigned sz, unsigned dc, bool nl);
    };

    // 数据集
    struct TRecordSetDM : public CSqlFunc::TRecordSet
    {
        CCtrlDM& m_own;
        ICtrlApiData& m_objCtrlD;
        KeepStmtDMPtr keepStmt;
        dhstmt& stmt;
        ulength row_num = 0;

        TRecordSetDM(CCtrlDM&, KeepStmtDMPtr, ICtrlApiData&);
        ~TRecordSetDM(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        // 获取字段信息
        bool GetColumnListMetaDataBeforeFetch(CSqlFunc::TFields &vecFeilds) override;
        // 下一条
        bool Next(void) override;
        // 添加值
        void AddVal(IKCJson& jsonRow, CSqlFunc::TField&) override;
        // 得到值
        CSqlFunc::TKcValVariant GetVal(CSqlFunc::TField&) const override;
    };

    // 数据库执行命令
    struct TDBCommandDM : public CSqlFunc::TDBCommand<TDmdpiException>
    {
        CCtrlDM& m_own;
        dhcon& conn;
        KeepStmtDMPtr keepStmt;
        dhstmt& stmt;
        ICtrlApiData& m_objCtrlD;
        TParmGrpDM m_parms;
        string m_act;
        string m_sql;
        string m_method;
        string &m_back;
        string m_nowTime;       // 时间标志，用于创建临时表、输出参数名等
        string m_parmTmpTab;    // 返回参数的临时表名

        TDBCommandDM(CCtrlDM&, dhcon&, KeepStmtDMPtr, ICtrlApiData&, string sAct, string sSQL, string sMethod, string &sBack);
        ~TDBCommandDM(void);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // SQL语句
        std::string GetSQL(void) override { return m_sql; }

        // 参数组
        TParmGrpDMBase& GetParms(void) override { return m_parms; }

        // 异常信息
        std::tuple<int, std::string, std::string> ParmExceptInfo(TDmdpiException& ex) override
        {
            return make_tuple(ex.m_code, ex.m_msg, ex.m_pos);
        }

        // 事务
        void TranBegin(void) override;
        void TranCommit(void) override;
        void TranRollback(void) override;

        // 预执行
        void PrepareSQL(void) override;

        // 执行
        unsigned int ExecuteUpdate(void) override;

        // 批量操作
        pair<int, string> ExecuteBatch(void) override;
        // 批量插入结束
        void BatchInsertEnd(bool) override;

        // 查询
        CSqlFunc::TRecordSetPtr ExecuteQuery(int &rows_affected) override;
        CSqlFunc::TRecordSetPtr NextResult(void) override;

        // 执行过程
        int ExecuteProcedures(void) override;

        // 单独执行sql语句
        void ExecSQL(const char*);
    };

private:
    // 宿主
    IKCSqlDM& m_own;
    // 名称
    string m_name = "connect1";
    // 达梦数据库环境
    dhenv m_henv = nullptr;
    // 数据库配置
    string m_srv = "127.0.0.1:5236", m_user = "SYSDBA", m_pass = "password", m_model = "sysdba";
    string m_charset = "GBK", m_charsetCfg = "";;
    std::atomic_uint m_connID;
    // 连接参数是否加密
    bool m_encryptConnStr = false;
};
