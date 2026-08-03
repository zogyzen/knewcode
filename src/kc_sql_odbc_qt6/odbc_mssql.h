#pragma once

#include "std.h"

class CCtrlODBC;
class TOdbcConnObj;
struct TParmGrpODBCmssql;

// 参数
struct TParmODBCmssql : public CSqlFunc::TParm
{
    TParmGrpODBCmssql& m_own;

    TParmODBCmssql(TParmGrpODBCmssql&, std::string = "", unsigned = 0);

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
    // 获取临时表数据集
    CSqlFunc::TRecordSetPtr GetDbSet(void) override;

    void SetDbTp(void) override {}

    void RegOutParam(void) override;
};
typedef CSqlFunc::TParmGrp<QSqlError> TParmGrpODBCmssqlBase;
struct TParmGrpODBCmssql : public TParmGrpODBCmssqlBase
{
    CCtrlODBC& m_own;
    TOdbcConnObj& m_connObj;
    QSqlQuery &m_qry;
    ICtrlApiData& m_objCtrlD;
    string m_act;

    TParmGrpODBCmssql(CCtrlODBC&, TOdbcConnObj&, QSqlQuery&, ICtrlApiData& ctrlD, string sAct);

    // 所属的服务
    IKCSql& Srv(void) override;

    // 控制器数据接口
    ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

    //  控制器 信息
    string ActInfo(void) override;

    // 创建参数
    CSqlFunc::TParmPtr MakePram(std::string = "", unsigned = 0) override;

    //  解析sql语句里的参数
    void ParseSqlParms(std::string sSQL, std::string sFmt = R"((?<!:):\w+)") override;

    // 异常信息
    string ParmExceptInfo(QSqlError&) override;
};

// 数据集
struct TRecordSetODBCmssqlBase : public CSqlFunc::TRecordSet
{
    CCtrlODBC& m_own;
    ICtrlApiData& m_objCtrlD;

    TRecordSetODBCmssqlBase(CCtrlODBC&, ICtrlApiData& ctrlD);
    ~TRecordSetODBCmssqlBase(void) override = default;

    // 控制器数据接口
    ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

    // sql实例
    virtual QSqlQuery& qry(void) = 0;
    virtual const QSqlQuery& qry(void) const = 0;
    // 获取字段信息
    void GetColumnListMetaData(CSqlFunc::TFields &vecFeilds) override;
    // 下一条
    bool Next(void) override { return qry().next(); }
    // 添加值
    void AddVal(IKCJson& jsonRow, CSqlFunc::TField&) override;
    // 得到值
    CSqlFunc::TKcValVariant GetVal(CSqlFunc::TField&) const override;
};
struct TRecordSetODBCmssql : public TRecordSetODBCmssqlBase
{
    TRecordSetODBCmssql(CCtrlODBC&, QSqlQuery&, ICtrlApiData& ctrlD);
    ~TRecordSetODBCmssql(void) override = default;

    // sql实例
    QSqlQuery &m_qry;
    QSqlQuery& qry(void) override { return m_qry; }
    const QSqlQuery& qry(void) const override { return m_qry; }
};
struct TRecordSetODBCmssqlAlone : public TRecordSetODBCmssqlBase
{
    TRecordSetODBCmssqlAlone(CCtrlODBC&, QSqlQuery&&, ICtrlApiData& ctrlD);
    ~TRecordSetODBCmssqlAlone(void) override = default;

    // sql实例
    QSqlQuery m_qry;
    QSqlQuery& qry(void) override { return m_qry; }
    const QSqlQuery& qry(void) const override { return m_qry; }
};

// 数据库执行命令
struct TDBCommandODBCmssql : public CSqlFunc::TDBCommand<QSqlError>
{
    CCtrlODBC& m_own;
    TOdbcConnObj& m_connObj;
    QSqlQuery &m_qry;
    ICtrlApiData& m_objCtrlD;
    TParmGrpODBCmssql m_parms;
    string m_act;
    string m_sql;
    string m_method;
    string &m_back;
    string m_nowTime;       // 时间标志，用于创建临时表、输出参数名等
    string m_parmTmpTab;    // 返回参数的临时表名

    TDBCommandODBCmssql(CCtrlODBC&, TOdbcConnObj&, QSqlQuery&, ICtrlApiData& ctrlD, string sAct, string sSQL, string sMethod, string &sBack);

    // 所属的服务
    IKCSql& Srv(void) override;

    // 控制器数据接口
    ICtrlApiData& CtrlD(void) override { return m_objCtrlD; }

    //  控制器 信息
    string ActInfo(void) override;

    // SQL语句
    std::string GetSQL(void) override { return m_sql; }

    // 参数组
    TParmGrpODBCmssqlBase& GetParms(void) override { return m_parms; }

    // 异常信息
    std::tuple<int, std::string, std::string> ParmExceptInfo(QSqlError& ex) override
    {
        return make_tuple(ex.type(), ex.text().toStdString(), "");
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
    CSqlFunc::TRecordSetPtr NextResult(void) override;

    // 统一获取参数值
    void MakeParmVals(void) override;
    void MakeParmVals(std::function<void(string, QVariant)>);

    // 执行过程
    int ExecuteProcedures(void) override;
};
