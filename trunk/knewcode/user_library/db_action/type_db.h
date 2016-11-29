#pragma once

namespace KC
{
    // 数据类型
    class CDBType : public IDBType
    {
    public:
        CDBType(IActionData&, IDBTypeList&, string, EDBType, int = 0);
        virtual CALL_TYPE ~CDBType() = default;

		// 类型
		virtual EDBType CALL_TYPE DBType(void) const;
        // 名称
		virtual const char* CALL_TYPE Name(void) const;
        // 数据类型列表
		virtual IDBTypeList& CALL_TYPE DBTypeList(void);

    public:
        // 源数据类型
        int SrcDBType(void) const;

    private:
        IActionData& m_act;
        IDBTypeList& m_list;
        string m_name;
        EDBType m_type = dvtInt;
        int m_srcType = 0;
    };
    typedef boost::shared_ptr<CDBType> TDBTypePtr;

    // 数据类型列表
    class CDBTypeList : public IDBTypeList
    {
    public:
        CDBTypeList(IActionData&);
        virtual CALL_TYPE ~CDBTypeList() = default;

		// 数量
		virtual int CALL_TYPE Count(void) const;
        // 存在
		virtual bool CALL_TYPE Exists(const char*) const;
        // 得到数据类型
		virtual IDBType& CALL_TYPE DBType(int);
		virtual IDBType& CALL_TYPE DBType(const char*);
		// 通过名称得到序号（0基准）
		virtual int CALL_TYPE Index(const char*) const;

    public:
        // 加类型
        void Add(TDBTypePtr);
        // 删类型
        void Del(int);
        void Clear(void);

    private:
        IActionData& m_act;
        // 类型列表
        vector<TDBTypePtr> m_DBTypeList;
        map<string, int> m_DBTypeNameList;
    };
}
