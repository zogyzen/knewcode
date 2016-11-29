#include "std.h"
#include "work_db_i.h"
#include "type_db.h"

////////////////////////////////////////////////////////////////////////////////
// CDBType类
CDBType::CDBType(IActionData& act, IDBTypeList& l, string n, EDBType t, int dt)
    : m_act(act), m_list(l), m_name(n), m_type(t), m_srcType(dt)
{
}

// 名称
const char* CDBType::Name(void) const
{
    return m_name.c_str();
}

// 类型
IDBType::EDBType CDBType::DBType(void) const
{
    return m_type;
}

// 数据类型列表
IDBTypeList& CDBType::DBTypeList(void)
{
    return m_list;
}

// 源数据类型
int CDBType::SrcDBType(void) const
{
    return m_srcType;
}

////////////////////////////////////////////////////////////////////////////////
// CDBTypeList类
CDBTypeList::CDBTypeList(IActionData& act) : m_act(act)
{
}

// 数量
int CDBTypeList::Count(void) const
{
    return (int)m_DBTypeList.size();
}

// 存在
bool CDBTypeList::Exists(const char* n) const
{
    return m_DBTypeNameList.end() != m_DBTypeNameList.find(n);
}

// 得到数据类型
IDBType& CDBTypeList::DBType(int i)
{
    if ((int)m_DBTypeList.size() <= i || i < 0)
    {
        string sError = "Field: " + lexical_cast<string>(i) + "\r\nArray index out of bounds";
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__);
        throw "Array_index_out_of_bounds";
    }
    return *m_DBTypeList[i].get();
}

IDBType& CDBTypeList::DBType(const char* n)
{
    return this->DBType(this->Index(n));
}

// 通过名称得到序号（0基准）
int CDBTypeList::Index(const char* n) const
{
    auto iter = m_DBTypeNameList.find(n);
    if (m_DBTypeNameList.end() == iter)
    {
        string sError = string("Field [") + n + "] don't exists";
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__);
        throw "Don_t_exists_field_";
    }
    return iter->second;
}

// 加值
void CDBTypeList::Add(TDBTypePtr t)
{
    if (this->Exists(t->Name()))
    {
        string sError = string("Field [") + t->Name() + "] redefine";
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__, t->Name());
        throw "Repeating_Field_";
    }
    m_DBTypeNameList.insert(make_pair(string(t->Name()), (int)m_DBTypeList.size()));
    m_DBTypeList.push_back(t);
}

// 删值
void CDBTypeList::Del(int i)
{
    if ((int)m_DBTypeList.size() <= i || i < 0)
    {
        string sError = "Field: " + lexical_cast<string>(i) + "\r\nArray index out of bounds";
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__);
        throw "Array_index_out_of_bounds";
    }
    auto iter = m_DBTypeList.begin() + i;
    for (auto it = iter; m_DBTypeList.end() != it; ++it)
    {
        string sName = (*it)->Name();
        auto itName = m_DBTypeNameList.find(sName);
        if (m_DBTypeNameList.end() != itName) --(itName->second);
    }
    m_DBTypeList.erase(iter);
}
void CDBTypeList::Clear(void)
{
    m_DBTypeList.clear();
    m_DBTypeNameList.clear();
}
