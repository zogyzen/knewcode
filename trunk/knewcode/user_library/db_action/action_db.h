#pragma once

namespace KC
{
    class CDBAction : public IDBAction
    {
    public:
        CDBAction(IActionData&, const char*, const char*, const char*);
        virtual CALL_TYPE ~CDBAction() = default;

        // ��ѯ����
		virtual IDBTable& CALL_TYPE Select(const char*, const char* = "TableName");
		virtual IDBTable& CALL_TYPE Select(const char*, IDBParms&, const char* = "TableName");
        // ��ȡֵ
		virtual const char* CALL_TYPE GetValue(const char*);
		virtual const char* CALL_TYPE GetValue(const char*, IDBParms&);
		// �������ݼ�
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, const char* = "TableName");
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, IDBParms&, const char* = "TableName");
		virtual IDBSet& CALL_TYPE CreateDBSet(IDBTable&);
		virtual void CALL_TYPE ReleaseDBSet(IDBSet*&);
		// ִ����䣨���롢���¡�ɾ����
		virtual int CALL_TYPE ExecSQL(const char*);
		virtual int CALL_TYPE ExecSQL(const char*, IDBParms&);
		// ��ռ������
        virtual int CALL_TYPE ExecSQLLck(const char*);
		virtual int CALL_TYPE ExecSQLLck(const char*, IDBParms&);

    private:
        // ִ�����
        int ExecSQLBase(const char*, IDBParms* = nullptr);
        // ��ѯ����
		CDBTable& SelectBase(const char*, const char* = "TableName", IDBParms* = nullptr);
        // ��ȡֵ
		const char* GetValueBase(const char*, IDBParms* = nullptr);
		// ת���������ݼ�
        CDBSet& CreateDBSetBase(CDBTable&, string);

    private:
        IActionData& m_act;
        string m_name, m_drive, m_connstr, m_LckName, m_GetValue = "";
        // ���ݱ��б�
        typedef boost::shared_ptr<CDBTable> TDBTablePtr;
        typedef vector<TDBTablePtr> TDBTableList;
        TDBTableList m_DBTableList;
        // ���ݼ��б�
        typedef boost::shared_ptr<CDBSet> TDBSetPtr;
        typedef map<string, TDBSetPtr> TDBSetList;
        TDBSetList m_DBSetList;
        int m_DBSetMaxID = 0;
    };
}
