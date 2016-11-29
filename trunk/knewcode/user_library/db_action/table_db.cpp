#include "std.h"
#include "work_db_i.h"
#include "type_db.h"
#include "value_db.h"
#include "row_db.h"
#include "table_db.h"

////////////////////////////////////////////////////////////////////////////////
// CDBTable类
CDBTable::CDBTable(IActionData& act, TKCDBSetPtr rs, TKCDBSessionPtr c, string n)
    : m_act(act), m_name(n), m_rsPos(rs->begin()), m_Fields(act), m_Row(act, *this)
{
    m_rs.push_back(rs);
    m_conn.push_back(c);
}

CDBTable::~CDBTable()
{
    m_rs.clear();
    if (!m_conn.empty()) m_conn[0]->close();
    m_conn.clear();
}

// 名称
const char* CDBTable::Name(void)
{
    return m_name.c_str();
}

// 字段列表
IDBFieldList& CDBTable::Fields(void)
{
    return m_Fields;
}

// 获取当前记录行
IDBRow& CDBTable::Row(void)
{
    return m_Row;
}

// 当前记录行
int CDBTable::RowIndex(void) const
{
    return m_RowIndex;
}

// 下条记录
bool CDBTable::Next(void)
{
    // 初始
    if (m_RowIndex < 0)
    {
        if (this->Eof()) return false;
        this->LoadFields();
        this->LoadRow();
        m_RowIndex = 0;
        return true;
    }
    // 下条
    if (!this->Eof()) ++m_rsPos;
    if (!this->Eof()) this->LoadRow(), ++m_RowIndex;
    else
    {
        m_rs.clear();
        if (!m_conn.empty()) m_conn[0]->close();
        m_conn.clear();
    }
    return !this->Eof();
}

// 判断是否结束
bool CDBTable::Eof(void)
{
    return m_rs.empty() || m_rs[0]->end() == m_rsPos;
}

// 得到源库的数据行
row& CDBTable::GetSrcRow(void)
{
    if (this->Eof()) throw "Don_t_exists_data_";
    return *m_rsPos;
}

// 加载数据
void CDBTable::LoadFields(void)
{
    if (!this->Eof())
    {
        // 数据库行数据
        row& dr = *m_rsPos;
        // 初始化字段列表
        m_Fields.Clear();
        for (int i = 0, c = (int)dr.size(); i < c; ++i)
        {
            // lambda表达式
            auto TranDBType = [](data_type dbTp) -> IDBType::EDBType
            {
                if (dt_integer == dbTp)
                    return IDBType::dvtInt;
                else if (dt_long_long == dbTp || dt_unsigned_long_long == dbTp)
                    return IDBType::dvtBigInt;
                else if (dt_double == dbTp)
                    return IDBType::dvtDouble;
                else if (dt_date == dbTp)
                    return IDBType::dvtDatetime;
                else
                    return IDBType::dvtString;
            };
            // 字段信息
            column_properties const& col = dr.get_properties(i);
            string sName = col.get_name();
            soci::data_type srcTp = col.get_data_type();
            IDBType::EDBType dType = TranDBType(srcTp);
            // 创建数据字段
            CDBType* pField = new CDBType(m_act, m_Fields, sName, dType, (int)srcTp);
            TDBTypePtr FieldPtr(pField);
            // 保存字段
            m_Fields.Add(FieldPtr);
        }
    }
}

void CDBTable::LoadRow(void)
{
    if (!this->Eof())
    {
        m_Row.Clear();
        // 数据库行数据
        row& dr = *m_rsPos;
        // 数据行
        m_Row.Clear();
        // 循环每个字段
        for (int i = 0, j = m_Fields.Count(); i < j; ++i)
        {
            // 字段类型
            IDBType& DBType = m_Fields.DBType(i);
            // 添加字段值
            m_Row.Add(DBType, dr);
        }
    }
}
