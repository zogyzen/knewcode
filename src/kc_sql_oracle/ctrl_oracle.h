#pragma once

#include "std.h"

class CCtrlOracle : public IKCController
{
public:
    CCtrlOracle(IKCSqlOracle&, string, property_tree::ptree&);
    ~CCtrlOracle() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

protected:
    // 获取类型名
    static CSqlFunc::EDataType GetTypeName(int sqlt);

protected:
    // 页面活动对象（用于保存数据库连接，在整个页面执行期间有效）
    struct TOraConnObj final : public IActionData::TActObj
    {
        CCtrlOracle& m_own;
        Connection *m_conn = nullptr;
        const unsigned m_connid = 0;
        // 临时对象（临时表、临时视图、临时函数，等等）
        map<string, string> mapTmpObjs;
        // 临时对象计数
        std::atomic_uint m_tmpID;

        TOraConnObj(CCtrlOracle& own, unsigned connid);
        void Release(void) override;
    };
    friend struct TOraConnObj;

    // 保持数据库执行实例
    struct KeepStmtOra
    {
        Connection &conn;
        Statement &stmt;
        KeepStmtOra(Connection &c, Statement &s) : conn(c), stmt(s) {}
        ~KeepStmtOra(void) { conn.terminateStatement(&stmt); }
    };
    typedef std::shared_ptr<KeepStmtOra> KeepStmtOraPtr;

    // 参数
    struct TParmGrpOra;
    struct TParmOra : public CSqlFunc::TParm
    {
        TParmGrpOra &m_own;
        Type eDbTP = OCCISTRING;

        TParmOra(TParmGrpOra& own, string = "", unsigned = 0);

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
        string GetDate(string = "YYYY-MM-DD") override;
        string GetDateTime(string = "YYYY-MM-DD HH24:MI:SS") override;
        int GetInt(int = 0) override;
        double GetNumber(double = 0) override;
        // 获取游标数据集
        CSqlFunc::TRecordSetPtr GetDbSet(void) override;

        void SetDbTp(void) override;

        void RegOutParam(void) override;

        static void SetClob(Connection&, Statement&, unsigned iPos, string strVal);
        static string GetClob(Statement&, unsigned iPos, string sDef = "");
    };
    typedef CSqlFunc::TParmGrp<SQLException> TParmGrpOraBase;
    struct TParmGrpOra : public TParmGrpOraBase
    {
        CCtrlOracle& m_ctrl;
        IKCSqlOracle& m_own;
        KeepStmtOraPtr keepStmt;
        Connection &conn;
        Statement &stmt;
        ICtrlApiData& m_objCtrlD;
        string m_act;

        TParmGrpOra(CCtrlOracle& ctrl, KeepStmtOraPtr st, ICtrlApiData& ctrlD, string sAct, bool bMultiParm = false);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // 创建参数
        CSqlFunc::TParmPtr MakePram(std::string = "", unsigned = 0) override;

        // 异常信息
        string ParmExceptInfo(SQLException&) override;
    };

    // 数据集
    struct TRecordSetOra : public CSqlFunc::TRecordSet
    {
        CCtrlOracle& m_ctrl;
        KeepStmtOraPtr keepStmt;
        Statement &stmt;
        ResultSet &rset;
        ICtrlApiData& m_objCtrlD;

        TRecordSetOra(CCtrlOracle& ctrl, KeepStmtOraPtr st, ResultSet& rs, ICtrlApiData& ctrlD);
        ~TRecordSetOra(void) override;

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
    struct TDBCommandOra : public CSqlFunc::TDBCommand<SQLException>
    {
        IKCSqlOracle& m_own;
        CCtrlOracle& m_ctrl;
        KeepStmtOraPtr keepStmt;
        Connection &conn;
        Statement &stmt;
        ICtrlApiData& m_objCtrlD;
        string m_act, m_actName;
        string m_method;
        TOraConnObj& m_connObj;
        bool m_batchIsProc = false;
        TParmGrpOra m_parms;

        TDBCommandOra(CCtrlOracle& ctrl, KeepStmtOraPtr st, ICtrlApiData& ctrlD, string sAct, string sActName, string sMethod, TOraConnObj&);

        // 所属的服务
        IKCSql& Srv(void) override;

        // 控制器数据接口
        ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

        //  控制器 信息
        string ActInfo(void) override;

        // SQL语句
        std::string GetSQL(void) override { return stmt.getSQL(); }

        // 参数组
        TParmGrpOraBase& GetParms(void) override { return m_parms; }

        // 异常信息
        std::tuple<int, std::string, std::string> ParmExceptInfo(SQLException& ex) override
        {
            return make_tuple(ex.getErrorCode(), ex.getMessage(), "");
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

        // 查询
        CSqlFunc::TRecordSetPtr ExecuteQuery(int &rows_affected) override;

        // 执行过程
        int ExecuteProcedures(void) override;
        // 针对DDL语句，创建临时参数包
        void CreateTmpParmPackage(void);
        // 创建临时包
        void CreateTmpPackage(ICtrlApiData&);
        // 临时对象的随机名称
        string GetTmpObjRandName(string sName);
    };

protected:
    // 获取配置
    string GetConfig(property_tree::ptree&, string sAttr, string sDef = "");
    // 创建oracle环境
    Environment& CreateOracleEnv(property_tree::ptree&);
    // 创建连接池
    StatelessConnectionPool* CreateConnPool(property_tree::ptree&);

private:
    // 宿主
    IKCSqlOracle& m_own;
    // 名称
    string m_name = "connect1";
    // 连接串是否加密
    bool m_encryptConnStr = false;
    // 数据库链接串
    string m_srv = "(DESCRIPTION=(ADDRESS=(PROTOCOL=tcp)(HOST=127.0.0.1)(PORT=1521))(CONNECT_DATA=(SERVICE_NAME=orcl)))",
        m_user = "system", m_pass = "manager";
    // 数据库配置
    bool m_usePool = false;
    int m_maxConnNum = 512;
    double m_version = 19;
    // 单字节字符集、宽字节字符集
    const string m_Charset = "AL32UTF8", m_NCharset = "AL32UTF8";
    // oracle数据库环境
    Environment &m_env;
    // 链接池
    StatelessConnectionPool *m_connPool = nullptr;
    // 链接编号
    std::atomic_uint m_connID;

    friend struct TRecordSetOra;
};
