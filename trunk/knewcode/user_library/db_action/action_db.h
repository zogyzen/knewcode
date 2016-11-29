#pragma once

namespace KC
{
    class CDBAction : public IDBAction
    {
    public:
        CDBAction(IActionData&, const char*, const char*, const char*);
        virtual CALL_TYPE ~CDBAction() = default;

        // 查询数据
		virtual IDBTable& CALL_TYPE Select(const char*, const char* = "TableName");
		virtual IDBTable& CALL_TYPE Select(const char*, IDBParms&, const char* = "TableName");
        // 获取值
		virtual const char* CALL_TYPE GetValue(const char*);
		virtual const char* CALL_TYPE GetValue(const char*, IDBParms&);
		// 独立数据集
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, const char* = "TableName");
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, IDBParms&, const char* = "TableName");
		virtual IDBSet& CALL_TYPE CreateDBSet(IDBTable&);
		virtual void CALL_TYPE ReleaseDBSet(IDBSet*&);
		// 执行语句（插入、更新、删除）
		virtual int CALL_TYPE ExecSQL(const char*);
		virtual int CALL_TYPE ExecSQL(const char*, IDBParms&);
		// 独占进程锁
        virtual int CALL_TYPE ExecSQLLck(const char*);
		virtual int CALL_TYPE ExecSQLLck(const char*, IDBParms&);

    private:
        // 执行语句
        int ExecSQLBase(const char*, IDBParms* = nullptr);
        // 查询数据
		CDBTable& SelectBase(const char*, const char* = "TableName", IDBParms* = nullptr);
        // 获取值
		const char* GetValueBase(const char*, IDBParms* = nullptr);
		// 转换独立数据集
        CDBSet& CreateDBSetBase(CDBTable&, string);

    private:
        IActionData& m_act;
        string m_name, m_drive, m_connstr, m_LckName, m_GetValue = "";
        // 数据表列表
        typedef boost::shared_ptr<CDBTable> TDBTablePtr;
        typedef vector<TDBTablePtr> TDBTableList;
        TDBTableList m_DBTableList;
        // 数据集列表
        typedef boost::shared_ptr<CDBSet> TDBSetPtr;
        typedef map<string, TDBSetPtr> TDBSetList;
        TDBSetList m_DBSetList;
        int m_DBSetMaxID = 0;
    };
}
