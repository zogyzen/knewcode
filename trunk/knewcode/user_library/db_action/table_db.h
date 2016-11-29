#pragma once

namespace KC
{
    class CDBTable : public IDBTable
    {
    public:
        CDBTable(IActionData&, TKCDBSetPtr, TKCDBSessionPtr, string = "TableName");
        virtual CALL_TYPE ~CDBTable();

        // ����
		virtual const char* CALL_TYPE Name(void);
		// �ֶ��б�
		virtual IDBFieldList& CALL_TYPE Fields(void);
		// ��ȡ��ǰ��¼��
		virtual IDBRow& CALL_TYPE Row(void);
		// ��ǰ��¼��
		virtual int CALL_TYPE RowIndex(void) const;
		// ������¼
		virtual bool CALL_TYPE Next(void);
		// �ж��Ƿ����
		virtual bool CALL_TYPE Eof(void);

    public:
        // �õ�Դ���������
        row& GetSrcRow(void);

    private:
        // ��������
        void LoadFields(void);
        void LoadRow(void);

    private:
        IActionData& m_act;
        string m_name;
        // ��ǰλ��
        TKCDBRowPos m_rsPos;
        int m_RowIndex = -1;
        // �������ӣ���ʱ���ԣ�
        vector<TKCDBSessionPtr> m_conn;
        // ���ݼ�
        vector<TKCDBSetPtr> m_rs;
        // �ֶ�
        CDBTypeList m_Fields;
        // ��¼��
        CDBRow m_Row;
    };
}
