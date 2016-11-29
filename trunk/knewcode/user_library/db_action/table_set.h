#pragma once

namespace KC
{
    class CDBSet : public IDBSet
    {
    public:
        CDBSet(IActionData&, int, string);
        virtual CALL_TYPE ~CDBSet() = default;

        // 名称
		virtual const char* CALL_TYPE Name(void);
		// 字段列表
		virtual IDBFieldList& CALL_TYPE Fields(void);
        // 记录数
		virtual int CALL_TYPE RowCount(void);
		virtual bool CALL_TYPE Empty(void);
		// 获取记录行
		virtual IDBRow& CALL_TYPE Row(int);
		virtual IDBRow& CALL_TYPE Row(void);
		virtual int CALL_TYPE RowIndex(void) const;
		// 判断是否结束
		virtual bool CALL_TYPE Eof(void);
		// 调整当前记录行位置
		virtual bool CALL_TYPE Prev(void);
		virtual bool CALL_TYPE Next(void);
		virtual bool CALL_TYPE Move(int);
		virtual IDBRow& CALL_TYPE First(void);
		virtual IDBRow& CALL_TYPE Last(void);

    public:
        // 初始化数据
        void InitDB(CDBTable&);
        // 唯一标志
		string Tag(void);

    private:
        IActionData& m_act;
        // 字段和行列表
        CDBTypeList m_Fields;
        // 编号、名称
        int m_id;
        string m_name, m_tag;
        // 数据行
        typedef boost::shared_ptr<CDBRow> TDBRowPtr;
        typedef vector<TDBRowPtr> TDBRows;
        TDBRows m_Rows;
        // 当前行
        mutable int m_RowIndex = -1;

    };
}
