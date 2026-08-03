#pragma once

#include "std.h"

class CCtrlODBC : public IKCController
{
public:
    CCtrlODBC(IKCSqlODBC&, string, property_tree::ptree&);
    ~CCtrlODBC() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

public:
    // 所属的服务
    IKCSql& Srv(void) { return m_own; }
    IKCSqlODBC& own(void) { return m_own; }

protected:
    // 数据库连接参数
    void GetConnParm(property_tree::ptree&);
    // 获取类型名
    static CSqlFunc::EDataType GetTypeName(int tp);

private:
    // 宿主
    IKCSqlODBC& m_own;
    string m_name = "connect1";
    string m_connStr = "Driver={Sql Server};Server=.;Database=dbname;Uid=sa;Pwd=pass";
    bool m_encryptConnStr = false;
    string m_dbType = "mssql";
    bool m_parmCaseSensitive = false;
    std::atomic_uint m_connID;

    friend class TOdbcConnObj;
    friend struct TRecordSetODBCmssql;
};

// 页面活动对象（用于保存数据库连接，在整个页面执行期间有效）
class TOdbcConnObj final : public IActionData::TActObj
{
public:
    CCtrlODBC& m_own;
    string m_connName;
    QSqlDatabase m_odbc;
    const unsigned m_connid = 0;


    TOdbcConnObj(CCtrlODBC& own, string sConnName, unsigned connid);
    void Release(void) override;
};
