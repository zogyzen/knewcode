#pragma once

namespace KC
{
    class CDBRow : public IDBRow
    {
    public:
        CDBRow(IActionData&, IDBRowsBase&);
        virtual CALL_TYPE ~CDBRow() = default;

		// 得到值
		virtual IDBValue& CALL_TYPE Value(const char*);
		virtual IDBValue& CALL_TYPE Value(int);
        // 指派到参数
		virtual bool CALL_TYPE AssignTo(IDBParms&);
        // 数据表
		virtual IDBRowsBase& CALL_TYPE Table(void);

    public:
        // 添加值
		IDBValue& Add(IDBType&, row&);
		// 移除值
		void Remove(int);
		void Clear(void);

    protected:
        // 数据类型列表
		virtual IDBTypeList& CALL_TYPE DBTypeList(void);

    private:
        IActionData& m_act;
        IDBRowsBase& m_table;
        // 值列表
        CDBValueList m_values;
    };
}
