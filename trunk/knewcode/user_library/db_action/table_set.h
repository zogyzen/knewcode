#pragma once

namespace KC
{
    class CDBSet : public IDBSet
    {
    public:
        CDBSet(IActionData&, int, string);
        virtual CALL_TYPE ~CDBSet() = default;

        // ����
		virtual const char* CALL_TYPE Name(void);
		// �ֶ��б�
		virtual IDBFieldList& CALL_TYPE Fields(void);
        // ��¼��
		virtual int CALL_TYPE RowCount(void);
		virtual bool CALL_TYPE Empty(void);
		// ��ȡ��¼��
		virtual IDBRow& CALL_TYPE Row(int);
		virtual IDBRow& CALL_TYPE Row(void);
		virtual int CALL_TYPE RowIndex(void) const;
		// �ж��Ƿ����
		virtual bool CALL_TYPE Eof(void);
		// ������ǰ��¼��λ��
		virtual bool CALL_TYPE Prev(void);
		virtual bool CALL_TYPE Next(void);
		virtual bool CALL_TYPE Move(int);
		virtual IDBRow& CALL_TYPE First(void);
		virtual IDBRow& CALL_TYPE Last(void);

    public:
        // ��ʼ������
        void InitDB(CDBTable&);
        // Ψһ��־
		string Tag(void);

    private:
        IActionData& m_act;
        // �ֶκ����б�
        CDBTypeList m_Fields;
        // ��š�����
        int m_id;
        string m_name, m_tag;
        // ������
        typedef boost::shared_ptr<CDBRow> TDBRowPtr;
        typedef vector<TDBRowPtr> TDBRows;
        TDBRows m_Rows;
        // ��ǰ��
        mutable int m_RowIndex = -1;

    };
}
