#pragma once

namespace KC
{
    class CDBTable : public IDBTable
    {
    public:
        CDBTable(IActionData&, TKCDBSetPtr, TKCDBSessionPtr, string = "TableName");
        virtual CALL_TYPE ~CDBTable();

        // 名称
		virtual const char* CALL_TYPE Name(void);
		// 字段列表
		virtual IDBFieldList& CALL_TYPE Fields(void);
		// 获取当前记录行
		virtual IDBRow& CALL_TYPE Row(void);
		// 当前记录行
		virtual int CALL_TYPE RowIndex(void) const;
		// 下条记录
		virtual bool CALL_TYPE Next(void);
		// 判断是否结束
		virtual bool CALL_TYPE Eof(void);

    public:
        // 得到源库的数据行
        row& GetSrcRow(void);

    private:
        // 加载数据
        void LoadFields(void);
        void LoadRow(void);

    private:
        IActionData& m_act;
        string m_name;
        // 当前位置
        TKCDBRowPos m_rsPos;
        int m_RowIndex = -1;
        // 数据连接（临时测试）
        vector<TKCDBSessionPtr> m_conn;
        // 数据集
        vector<TKCDBSetPtr> m_rs;
        // 字段
        CDBTypeList m_Fields;
        // 记录行
        CDBRow m_Row;
    };
}
