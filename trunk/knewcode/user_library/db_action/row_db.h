#pragma once

namespace KC
{
    class CDBRow : public IDBRow
    {
    public:
        CDBRow(IActionData&, IDBRowsBase&);
        virtual CALL_TYPE ~CDBRow() = default;

		// �õ�ֵ
		virtual IDBValue& CALL_TYPE Value(const char*);
		virtual IDBValue& CALL_TYPE Value(int);
        // ָ�ɵ�����
		virtual bool CALL_TYPE AssignTo(IDBParms&);
        // ���ݱ�
		virtual IDBRowsBase& CALL_TYPE Table(void);

    public:
        // ���ֵ
		IDBValue& Add(IDBType&, row&);
		// �Ƴ�ֵ
		void Remove(int);
		void Clear(void);

    protected:
        // ���������б�
		virtual IDBTypeList& CALL_TYPE DBTypeList(void);

    private:
        IActionData& m_act;
        IDBRowsBase& m_table;
        // ֵ�б�
        CDBValueList m_values;
    };
}
