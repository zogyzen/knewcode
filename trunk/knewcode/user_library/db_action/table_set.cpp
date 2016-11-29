#include "std.h"
#include "work_db_i.h"
#include "type_db.h"
#include "value_db.h"
#include "row_db.h"
#include "table_db.h"
#include "table_set.h"

////////////////////////////////////////////////////////////////////////////////
// CDBSet类
CDBSet::CDBSet(IActionData& act, int id, string n)
    : m_act(act), m_Fields(m_act), m_id(id), m_name(n)
    , m_tag(n + "_" + lexical_cast<string>(m_id) + "_" + lexical_cast<string>(rand()))
{
}

// 初始化数据
void CDBSet::InitDB(CDBTable& tab)
{
    if (tab.RowIndex() < 0) tab.Next();
    // 初始化字段列表
    m_Fields.Clear();
    for (int i = 0, c = tab.Fields().Count(); i < c; ++i)
    {
        CDBType& src = dynamic_cast<CDBType&>(tab.Fields().DBType(i));
        // 创建数据字段
        CDBType* pField = new CDBType(m_act, m_Fields, src.Name(), src.DBType(), src.SrcDBType());
        TDBTypePtr FieldPtr(pField);
        // 保存字段
        m_Fields.Add(FieldPtr);
    }
    // 初始化记录
    m_Rows.clear();
    if (!tab.Eof()) do
    {
        CDBRow *pRow = new CDBRow(m_act, *this);
        TDBRowPtr DrPtr(pRow);
        for (int i = 0, c = m_Fields.Count(); i < c; ++i)
        {
            // 字段类型
            IDBType& DBType = m_Fields.DBType(i);
            // 添加字段值
            pRow->Add(DBType, tab.GetSrcRow());
        }
        m_Rows.push_back(DrPtr);
    }
    while (tab.Next());
}

// 唯一标志
string CDBSet::Tag(void)
{
    return m_tag;
}

// 名称
const char* CDBSet::Name(void)
{
    return m_name.c_str();
}

// 字段列表
IDBFieldList& CDBSet::Fields(void)
{
    return m_Fields;
}

// 记录数
int CDBSet::RowCount(void)
{
    return (int)m_Rows.size();
}

bool CDBSet::Empty(void)
{
    return m_Rows.empty();
}

// 获取记录行
IDBRow& CDBSet::Row(int pos)
{
    if ((int)m_Rows.size() <= pos)
    {
        string sError = m_name + " row: " + lexical_cast<string>(pos) + "\r\nArray index out of bounds";
        m_act.GetActNow().WriteLogError(sError.c_str(), __FUNCTION__, m_name.c_str());
        throw "Array_index_out_of_bounds";
    }
    return *m_Rows[pos];
}

IDBRow& CDBSet::Row(void)
{
    if (m_RowIndex < 0) m_RowIndex = 0;
    return this->Row(m_RowIndex);
}

int CDBSet::RowIndex(void) const
{
    if (m_RowIndex < 0) m_RowIndex = 0;
    return m_RowIndex;
}

// 判断是否结束
bool CDBSet::Eof(void)
{
    return m_RowIndex >= this->RowCount() - 1;
}

// 调整当前记录行位置
bool CDBSet::Prev(void)
{
    return m_RowIndex > 0 ? --m_RowIndex, true : false;
}

bool CDBSet::Next(void)
{
    return m_RowIndex < (int)m_Rows.size() - 1 ? ++m_RowIndex, true : false;
}

bool CDBSet::Move(int pos)
{
    return pos < (int)m_Rows.size() ? (m_RowIndex = pos), true : false;
}

IDBRow& CDBSet::First(void)
{
    m_RowIndex = 0;
    return this->Row();
}

IDBRow& CDBSet::Last(void)
{
    m_RowIndex = this->RowCount() - 1;
    return this->Row();
}
