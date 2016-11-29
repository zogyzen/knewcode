#include "std.h"
#include "work_db_i.h"
#include "type_db.h"
#include "value_db.h"
#include "row_db.h"
#include "parms_db.h"

////////////////////////////////////////////////////////////////////////////////
// CDBRow类
CDBRow::CDBRow(IActionData& act, IDBRowsBase& tab) : m_act(act), m_table(tab), m_values(m_act, m_table.Fields())
{
}

// 数据类型列表
IDBTypeList& CDBRow::DBTypeList(void)
{
    return m_values.DBTypeList();
}

// 得到值
IDBValue& CDBRow::Value(const char* n)
{
    return m_values.Value(n);
}

IDBValue& CDBRow::Value(int i)
{
    return m_values.Value(i);
}

// 指派到参数
bool CDBRow::AssignTo(IDBParms& p)
{
    p.Clear();
    IDBFieldList& fields = m_table.Fields();
    // 循环每个字段
    for (int i = 0, j = fields.Count(); i < j; ++i)
    {
        // 类型和值
        IDBType& dbType = fields.DBType(i);
        IDBValue& dbVal = m_values.Value(i);
        // 添加参数
        if (IDBType::dvtInt == dbType.DBType())
            p.Add(dbType.Name(), dbVal.AsInt());
        else if (IDBType::dvtDouble == dbType.DBType())
            p.Add(dbType.Name(), dbVal.AsDouble());
        else if (IDBType::dvtString == dbType.DBType())
            p.Add(dbType.Name(), dbVal.AsString());
    }
    return true;
}

// 数据表
IDBRowsBase& CDBRow::Table(void)
{
    return m_table;
}

// 添加值
IDBValue& CDBRow::Add(IDBType& t, row& r)
{
    // 新值
    CDBValue* pVal = new CDBValue(m_act, t, m_values);
    TDBValuePtr ValPtr(pVal);
    // 设置新值
    if (r.get_indicator(t.Name()) == i_ok)
        try
        {
            if (IDBType::dvtInt == t.DBType())
                pVal->SetInt(r.get<int>(t.Name()));
            else if (IDBType::dvtBigInt == t.DBType())
                pVal->SetInt(r.get<long long>(t.Name()));
            else if (IDBType::dvtDouble == t.DBType())
                pVal->SetDouble(r.get<double>(t.Name()));
            else if (IDBType::dvtDatetime == t.DBType())
                pVal->SetAny(boost::any(r.get<std::tm>(t.Name())));
            else if (IDBType::dvtString == t.DBType())
                pVal->SetString(r.get<string>(t.Name()).c_str());
        }
        catch (std::bad_cast& ex)
        {
            string sError = string(m_table.Name()) + "." + t.Name() + "["
                + lexical_cast<string>((int)t.DBType()) + ":" + lexical_cast<string>(dynamic_cast<CDBType&>(t).SrcDBType())
                + "]\r\n" + ex.what();
            m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__, typeid(ex).name());
            throw;
        }
    // 添加值
    m_values.Add(ValPtr);
    return *pVal;
}

// 移除字段
void CDBRow::Remove(int i)
{
    m_values.Del(i);
}

void CDBRow::Clear(void)
{
    m_values.Clear();
}
