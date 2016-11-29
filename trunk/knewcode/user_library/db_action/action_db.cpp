#include "std.h"
#include "work_db_i.h"
#include "type_db.h"
#include "value_db.h"
#include "row_db.h"
#include "table_db.h"
#include "table_set.h"
#include "action_db.h"

////////////////////////////////////////////////////////////////////////////////
// 进程锁
constexpr char const g_DBLockName[] = "__KCPageDBWorkSQL19450903_";
typedef TProcessNameSharedLock<g_DBLockName> TSharedLock;
typedef TProcessNameUpgradLock<g_DBLockName> TUpgradLock;
typedef TProcessNameScopedLock<g_DBLockName> TScopedLock;

template<typename TLck>
class CKCLock
{
public:
    CKCLock(IActionData& act, string info, string pos, const char* lck)
        : m_act(act), m_info(info), m_pos(pos)
    {
        m_act.WriteLogDebug(("Lock - SQL: " + m_info).c_str(), m_pos.c_str());
        m_lck = new TLck(lck, posix_time::second_clock::local_time() + posix_time::seconds(m_act.GetRequestRespond().GetTimeOutSeconds()));
    }
    ~CKCLock()
    {
        delete m_lck;
        m_act.WriteLogDebug(("UnLock - SQL: " + m_info).c_str(), m_pos.c_str());
    }
    bool IsLock(void) { return nullptr != m_lck && m_lck->IsLock(); }

private:
    IActionData& m_act;
    string m_info, m_pos;
    TLck* m_lck = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// 参数
struct CKCDBParm
{
    CKCDBParm(IDBParms& v, session& s) : m_Vals(v), m_sql(s) {}
    CKCDBParm(const CKCDBParm& c) : m_Vals(c.m_Vals), m_sql(c.m_sql) {}
    IDBValueList& m_Vals;
    session& m_sql;
};

namespace soci
{
    template<> struct type_conversion<CKCDBParm>
    {
        typedef values base_type;
        static void from_base(values const & v, indicator ind, CKCDBParm & r)
        {
            // int i = v.get<int>(0);
            ind = i_ok;
        }
        static void to_base(CKCDBParm & r, values & v, indicator & ind)
        {
            ind = i_null;
            if (r.m_Vals.DBTypeList().Count() > 0)
            {
                for (int i = r.m_Vals.DBTypeList().Count() - 1; i >= 0; --i)
                {
                    IDBType& dbTp = r.m_Vals.DBTypeList().DBType(i);
                    if (dbTp.DBType() == IDBType::dvtInt)
                        v.set<int>(dbTp.Name(), r.m_Vals.Value(i).AsInt());
                    else if (dbTp.DBType() == IDBType::dvtDouble)
                        v.set<double>(dbTp.Name(), r.m_Vals.Value(i).AsDouble());
                    else if (dbTp.DBType() == IDBType::dvtString)
                        v.set<string>(dbTp.Name(), string(r.m_Vals.Value(i).AsString()));
                    /*
                    else if (dbTp.DBType() == IDBType::dvtBytes)
                    {
                        blob b(r.m_sql);
                        int cnt = 0;
                        const char* pBuf = r.m_Vals.Value(i).AsBytes(cnt);
                        b.append(pBuf, cnt);
                        //v.set<blob>(dbTp.Name(), b);
                    }
                    */
                }
                ind = i_ok;
            }
        }
    };

    /*
    template<> struct type_conversion<blob>
    {
        typedef int base_type;
        static void from_base(values const & v, indicator ind, base_type & r)
        {
            ind = i_ok;
        }
        static void to_base(values & v, base_type & r, indicator & ind)
        {
            ind = i_ok;
        }
    };
    */
}

////////////////////////////////////////////////////////////////////////////////
// CDBAction类
CDBAction::CDBAction(IActionData& act, const char* n, const char* d, const char* s)
    : m_act(act), m_name(n), m_drive(d), m_connstr(s), m_LckName(g_DBLockName + m_drive + "_" + m_name)
{
}

// 查询数据
IDBTable& CDBAction::Select(const char* s, const char* n)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] Select [" + n + "]: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 查询函数
    IDBTable* pTab = nullptr;
    auto TryFunc = [&]()
    {
        pTab = &this->SelectBase(s, n);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("Select(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return *pTab;
}

IDBTable& CDBAction::Select(const char* s, IDBParms& p, const char* n)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] Select [" + n + "]: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 查询函数
    IDBTable* pTab = nullptr;
    auto TryFunc = [&]()
    {
        pTab = &this->SelectBase(s, n, &p);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("Select(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return *pTab;
}

CDBTable& CDBAction::SelectBase(const char* s, const char* n, IDBParms* p)
{
    CDBTable *pTab = nullptr;
    string sInfo = string("Select [") + n + "]: " + s + "\r\n";
    try
    {
        m_act.WriteLogDebug(sInfo.c_str(), __FUNCTION__);
        // 查询
        session* pSql = new session;
        TKCDBSessionPtr SqlPtr(pSql);
        pSql->open(m_drive, m_connstr);
        TKCDBSet* pRows = nullptr;
        if (nullptr != p)
        {
            CKCDBParm DBParm(*p, *pSql);
            pRows = new TKCDBSet((pSql->prepare << s, use(DBParm)));
        }
        else
            pRows = new TKCDBSet(pSql->prepare << s);
        TKCDBSetPtr RowsPtr(pRows);
        // 创建表
        pTab = new CDBTable(m_act, RowsPtr, SqlPtr, n);
        TDBTablePtr TabPtr(pTab);
        m_DBTableList.push_back(TabPtr);
    }
    catch (soci_error& ex)
    {
        string sError = sInfo + ex.what();
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__, typeid(ex).name());
        throw;
    }
    return *pTab;
}

// 获取值
const char* CDBAction::GetValue(const char* s)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] GetValue: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 获取值
    const char* pVal = nullptr;
    auto TryFunc = [&]()
    {
        pVal = this->GetValueBase(s);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("GetValue(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return pVal;
}
const char* CDBAction::GetValue(const char* s, IDBParms& p)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] GetValue: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 获取值
    const char* pVal = nullptr;
    auto TryFunc = [&]()
    {
        pVal = this->GetValueBase(s, &p);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("GetValue(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return pVal;
}
const char* CDBAction::GetValueBase(const char* s, IDBParms* p)
{
    m_GetValue.clear();
    string sInfo = string("GetValueBase: ") + s + "\r\n";
    try
    {
        m_act.WriteLogDebug(sInfo.c_str(), __FUNCTION__);
        // 查询
        session sql;
        sql.open(m_drive, m_connstr);
        CAutoRelease _auto(boost::bind(&session::close, &sql));
        if (nullptr != p)
        {
            CKCDBParm DBParm(*p, sql);
            sql << s, into(m_GetValue), use(DBParm);
        }
        else
            sql << s, into(m_GetValue);
    }
    catch (soci_error& ex)
    {
        string sError = sInfo + ex.what();
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__, typeid(ex).name());
        throw;
    }
    return (m_GetValue.empty() || "" == m_GetValue) ? nullptr : m_GetValue.c_str();
}

// 独立数据集
IDBSet& CDBAction::SelectToSet(const char* s, const char* n)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] SelectToSet [" + n + "]: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 查询
    IDBSet* pSet = nullptr;
    auto TryFunc = [&]()
    {
        CDBTable& tab = this->SelectBase(s, n);
        pSet = &this->CreateDBSetBase(tab, n);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("SelectToSet(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return *pSet;
}
IDBSet& CDBAction::SelectToSet(const char* s, IDBParms& p, const char* n)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] SelectToSet [" + n + "]: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 查询
    IDBSet* pSet = nullptr;
    auto TryFunc = [&]()
    {
        CDBTable& tab = this->SelectBase(s, n, &p);
        pSet = &this->CreateDBSetBase(tab, n);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("SelectToSet(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return *pSet;
}
IDBSet& CDBAction::CreateDBSet(IDBTable& tab)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] CreateDBSet[" + tab.Name() + "]";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 转换
    IDBSet* pSet = nullptr;
    auto TryFunc = [&]()
    {
        pSet = &this->CreateDBSetBase(dynamic_cast<CDBTable&>(tab), tab.Name());
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("CreateDBSet(" + string(tab.Name()) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return *pSet;
}
CDBSet& CDBAction::CreateDBSetBase(CDBTable& tab, string n)
{
    CDBSet* pSet = new CDBSet(m_act, ++m_DBSetMaxID, n);
    TDBSetPtr SetPtr(pSet);
    pSet->InitDB(tab);
    m_DBSetList.insert(make_pair(pSet->Tag(), SetPtr));
    m_act.WriteLogDebug(("CreateD DBSet - " + pSet->Tag()).c_str(), __FUNCTION__);
    return *pSet;
}
void CDBAction::ReleaseDBSet(IDBSet*& tab)
{
    string sTag = "NULL";
    CDBSet* pSet = dynamic_cast<CDBSet*>(tab);
    if (nullptr != pSet)
    {
        auto iter = m_DBSetList.find(pSet->Tag());
        if (m_DBSetList.end() != iter)
        {
            sTag = pSet->Tag();
            m_DBSetList.erase(iter);
        }
    }
    tab = nullptr;
    m_act.WriteLogDebug(("Release DBSet - " + sTag).c_str(), __FUNCTION__);
}

// 执行语句（插入、更新、删除）
int CDBAction::ExecSQL(const char* s)
{
    string sInfo = "[" + m_drive + "_" + m_name + "] ExecSQL: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 转换
    int stt = 0;
    auto TryFunc = [&]()
    {
        stt = this->ExecSQLBase(s);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("ExecSQL(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return stt;
}

int CDBAction::ExecSQL(const char* s, IDBParms& p)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] ExecSQL: " + s + "\r\n";
    CKCLock<TSharedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 转换
    int stt = 0;
    auto TryFunc = [&]()
    {
        stt = this->ExecSQLBase(s, &p);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("ExecSQL(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return stt;
}

int CDBAction::ExecSQLLck(const char* s)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] ExecSQLLck: " + s + "\r\n";
    CKCLock<TScopedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 转换
    int stt = 0;
    auto TryFunc = [&]()
    {
        stt = this->ExecSQLBase(s);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("ExecSQLLck(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return stt;
}

int CDBAction::ExecSQLLck(const char* s, IDBParms& p)
{
    // 进程锁
    string sInfo = "[" + m_drive + "_" + m_name + "] ExecSQLLck: " + s + "\r\n";
    CKCLock<TScopedLock> lck(m_act, sInfo, __FUNCTION__, m_LckName.c_str());
    // 转换
    int stt = 0;
    auto TryFunc = [&]()
    {
        stt = this->ExecSQLBase(s, &p);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), ("ExecSQLLck(" + string(s) + ")").c_str());
    m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
    return stt;
}

int CDBAction::ExecSQLBase(const char* s, IDBParms* p)
{
    int iResult = 0;
    string sInfo = string("ExecSQL: ") + s + "\r\n";
    try
    {
        m_act.WriteLogDebug(sInfo.c_str(), __FUNCTION__);
        session sql;
        sql.open(m_drive, m_connstr);
        CAutoRelease _auto(boost::bind(&session::close, &sql));
        if (nullptr != p)
        {
            CKCDBParm DBParm(*p, sql);
            statement st = (sql.prepare << s, use(DBParm));
            // CAutoRelease _auto(boost::bind(&statement::clean_up, &st));
            st.execute(true);
            iResult = (int)st.get_affected_rows();
        }
        else
        {
            statement st = (sql.prepare << s);
            // CAutoRelease _auto(boost::bind(&statement::clean_up, &st));
            st.execute(true);
            iResult = (int)st.get_affected_rows();
        }
    }
    catch (soci_error& ex)
    {
        string sError = sInfo + ex.what();
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__, typeid(ex).name());
    }
    return iResult;
}
