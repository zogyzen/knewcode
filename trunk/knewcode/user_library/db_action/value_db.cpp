#include "std.h"
#include "work_db_i.h"
#include "value_db.h"

////////////////////////////////////////////////////////////////////////////////
// CDBValue类
CDBValue::CDBValue(IActionData& act, IDBType& t, IDBValueList& l)
    : m_act(act), m_type(t), m_list(l)
{
}

CDBValue::CDBValue(IActionData& act, IDBType& t, IDBValueList& l, boost::any v)
    : m_act(act), m_type(t), m_list(l), m_IsNULL(false), m_value(v)
{
}

CDBValue::CDBValue(IActionData& act, IDBType& t, IDBValueList& l, string v)
    : m_act(act), m_type(t), m_list(l), m_IsNULL(false), m_valueStr(v)
{
}

// 数据类型
IDBType& CDBValue::DBType(void)
{
    return m_type;
}

// 值
bool CDBValue::IsNULL(void) const
{
    return m_IsNULL;
}

void CDBValue::SetNULL(void)
{
    m_IsNULL = true;
}

int CDBValue::AsInt(void) const
{
    if (m_IsNULL) return 0;
    else if (m_type.DBType() == IDBType::dvtInt) return boost::any_cast<int>(m_value);
    else if (m_type.DBType() == IDBType::dvtBigInt) return (int)boost::any_cast<long long>(m_value);
    else if (m_type.DBType() == IDBType::dvtDouble) return (int)boost::any_cast<double>(m_value);
    else if (m_type.DBType() == IDBType::dvtDatetime) return this->TimeToInt(boost::any_cast<std::tm>(m_value));
    else return lexical_cast<int>(m_valueStr);
}

void CDBValue::SetInt(int v)
{
    if (m_type.DBType() == IDBType::dvtInt) m_value = v;
    else if (m_type.DBType() == IDBType::dvtBigInt) m_value = (long long)v;
    else if (m_type.DBType() == IDBType::dvtDouble) m_value = (double)v;
    else if (m_type.DBType() == IDBType::dvtString) m_valueStr = lexical_cast<string>(v);
    else if (m_type.DBType() == IDBType::dvtDatetime) m_value = this->IntToTime(v);
    m_IsNULL = false;
}

double CDBValue::AsDouble(void) const
{
    if (m_IsNULL) return 0;
    else if (m_type.DBType() == IDBType::dvtInt) return boost::any_cast<int>(m_value);
    else if (m_type.DBType() == IDBType::dvtBigInt) return boost::any_cast<long long>(m_value);
    else if (m_type.DBType() == IDBType::dvtDouble) return boost::any_cast<double>(m_value);
    else if (m_type.DBType() == IDBType::dvtDatetime) return (int)this->TimeToInt(boost::any_cast<std::tm>(m_value));
    else return lexical_cast<double>(m_valueStr);
}

void CDBValue::SetDouble(double v)
{
    if (m_type.DBType() == IDBType::dvtInt) m_value = (int)v;
    else if (m_type.DBType() == IDBType::dvtBigInt) m_value = (long long)v;
    else if (m_type.DBType() == IDBType::dvtDouble) m_value = v;
    else if (m_type.DBType() == IDBType::dvtString) m_valueStr = lexical_cast<string>(v);
    else if (m_type.DBType() == IDBType::dvtDatetime) m_value = this->IntToTime((int)v);
    m_IsNULL = false;
}

const char* CDBValue::AsString(void) const
{
    if (m_IsNULL) return "";
    else if (m_type.DBType() == IDBType::dvtInt) m_valueStr = lexical_cast<string>(boost::any_cast<int>(m_value));
    else if (m_type.DBType() == IDBType::dvtBigInt) m_valueStr = lexical_cast<string>(boost::any_cast<long long>(m_value));
    else if (m_type.DBType() == IDBType::dvtDouble) m_valueStr = lexical_cast<string>(boost::any_cast<double>(m_value));
    else if (m_type.DBType() == IDBType::dvtDatetime) m_valueStr = this->TimeToStr(boost::any_cast<std::tm>(m_value));
    return m_valueStr.c_str();
}

void CDBValue::SetString(const char* v)
{
    m_valueStr = "";
    if (nullptr != v) m_valueStr = string(v);
    if (m_type.DBType() == IDBType::dvtInt) m_value = lexical_cast<int>(m_valueStr);
    else if (m_type.DBType() == IDBType::dvtBigInt) m_value = lexical_cast<long long>(m_valueStr);
    else if (m_type.DBType() == IDBType::dvtDouble) m_value = lexical_cast<double>(m_valueStr);
    else if (m_type.DBType() == IDBType::dvtDatetime) m_value = this->StrToTime(m_valueStr);
    m_IsNULL = false;
}

const char* CDBValue::AsString(const char* s, const char* d) const
{
    const char* pStr = this->AsString();
    if (nullptr != pStr && nullptr != s && nullptr != d)
        m_valueStr = boost::locale::conv::between(pStr, d, s);
    return m_valueStr.c_str();
}

void CDBValue::SetString(const char* v, const char* s, const char* d)
{
    string str = "";
    if (nullptr != v && nullptr != s && nullptr != d)
        str = boost::locale::conv::between(v, d, s);
    this->SetString(str.c_str());
}

/*
const char* CDBValue::AsBytes(int& c) const
{
    if (m_IsNULL) throw "The_field_value_is_NULL_";
    c = (int)m_valueStr.size();
    return m_valueStr.c_str();
}

void CDBValue::SetBytes(const char* v, int c)
{
    if (m_type.DBType() != IDBType::dvtBytes) throw "The_field_type_don_t_match_";
    m_valueStr = CDBValue::MakeBytes(v, c);
    m_IsNULL = false;
}
*/

// 设置获取值
boost::any CDBValue::AsAny(void) const
{
    return m_value;
}

void CDBValue::SetAny(boost::any v)
{
    m_value = v;
    m_IsNULL = m_value.empty();
}

// 时间类型转换为字符串
string CDBValue::TimeToStr(const std::tm& tm) const
{
    ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::tm CDBValue::StrToTime(const string& s) const
{
    std::tm t = {};
    std::istringstream ss(s);
    ss >> std::get_time(&t, "%Y-%b-%d %H:%M:%S");
    return t;
}

int CDBValue::TimeToInt(const std::tm& tm) const
{
    ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d");
    return lexical_cast<int>(oss.str());
}

std::tm CDBValue::IntToTime(int v) const
{
    std::tm t = {};
    std::istringstream ss(lexical_cast<string>(v));
    ss >> std::get_time(&t, "%Y%m%d");
    return t;
}

// 数据值列表
IDBValueList& CDBValue::DBValueList(void)
{
    return m_list;
}

// 创建缓冲区
string CDBValue::MakeBytes(const char* v, int c)
{
    string sResult = "";
    if (nullptr != v && c > 0) sResult.append(v, c);
    else if (nullptr != v) sResult = string(v);
    return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// CDBValueList类
CDBValueList::CDBValueList(IActionData& act, IDBTypeList& t) : m_act(act), m_types(t)
{
}

// 数据类型列表
IDBTypeList& CDBValueList::DBTypeList(void)
{
    return m_types;
}

// 得到值
IDBValue& CDBValueList::Value(int i)
{
    if ((int)m_DBValueList.size() <= i || i < 0)
    {
        string sError = "Field: " + lexical_cast<string>(i) + "\r\nArray index out of bounds";
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__);
        throw "Array_index_out_of_bounds";
    }
    return *m_DBValueList[i].get();
}

IDBValue& CDBValueList::Value(const char* n)
{
    return this->Value(m_types.Index(n));
}

// 加值
void CDBValueList::Add(TDBValuePtr v)
{
    m_DBValueList.push_back(v);
}

// 删值
void CDBValueList::Del(int i)
{
    if ((int)m_DBValueList.size() <= i || i < 0)
    {
        string sError = "Field: " + lexical_cast<string>(i) + "\r\nArray index out of bounds";
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__);
        throw "Array_index_out_of_bounds";
    }
    m_DBValueList.erase(m_DBValueList.begin() + i);
}
void CDBValueList::Clear(void)
{
    m_DBValueList.clear();
}
