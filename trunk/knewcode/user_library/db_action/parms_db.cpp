#include "std.h"
#include "work_db_i.h"
#include "type_db.h"
#include "value_db.h"
#include "parms_db.h"

////////////////////////////////////////////////////////////////////////////////
// CDBParms类
CDBParms::CDBParms(IActionData& act) : m_act(act), m_types(m_act), m_values(m_act, m_types)
{
}

// 加参数
template<typename TVAL>
IDBValue& CDBParms::AddParm(const char* n, IDBType::EDBType t, TVAL v)
{
    // 新类型
    CDBType* pType = new CDBType(m_act, m_types, n, t);
    TDBTypePtr TypePtr(pType);
    // 新值
    CDBValue* pVal = new CDBValue(m_act, *pType, m_values, v);
    TDBValuePtr ValPtr(pVal);
    // 插入新类型和新值
    m_types.Add(TypePtr);
    m_values.Add(ValPtr);
    return *pVal;
}

// 数据类型列表
IDBTypeList& CDBParms::DBTypeList(void)
{
    return m_values.DBTypeList();
}

// 得到值
IDBValue& CDBParms::Value(const char* n)
{
    return m_values.Value(n);
}

IDBValue& CDBParms::Value(int i)
{
    return m_values.Value(i);
}

// 添加参数
IDBValue& CDBParms::Add(const char* n, int v)
{
    return this->AddParm(n, IDBType::dvtInt, boost::any(v));
}
IDBValue& CDBParms::Add(const char* n, double v)
{
    return this->AddParm(n, IDBType::dvtDouble, boost::any(v));
}
IDBValue& CDBParms::Add(const char* n, const char* v)
{
    return this->AddParm(n, IDBType::dvtString, CDBValue::MakeBytes(v));
}
IDBValue& CDBParms::Add(const char* n, const char* v, const char* s, const char* d)
{
    string str = "";
    if (nullptr != v && nullptr != s && nullptr != d)
        str = boost::locale::conv::between(v, d, s);
    return this->Add(n, str.c_str());
}
/*
IDBValue& CDBParms::Add(const char* n, const char* v, int c)
{
    return this->AddParm(n, IDBType::dvtString, CDBValue::MakeBytes(v, c));
}
*/

// 移除参数
bool CDBParms::Remove(int i)
{
    m_types.Del(i);
    m_values.Del(i);
    return true;
}
bool CDBParms::Remove(const char* n)
{
    return this->Remove(m_types.Index(n));
}

bool CDBParms::Clear(void)
{
    m_types.Clear();
    m_values.Clear();
    return true;
}
