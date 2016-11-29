#pragma once

namespace KC
{
    class CDBParms : public IDBParms
    {
    public:
        CDBParms(IActionData&);
        virtual CALL_TYPE ~CDBParms() = default;

        // 数据类型列表
		virtual IDBTypeList& CALL_TYPE DBTypeList(void);
		// 得到值
		virtual IDBValue& CALL_TYPE Value(const char*);
		virtual IDBValue& CALL_TYPE Value(int);
        // 添加参数
		virtual IDBValue& CALL_TYPE Add(const char*, int);
		virtual IDBValue& CALL_TYPE Add(const char*, double);
		virtual IDBValue& CALL_TYPE Add(const char*, const char*);
		virtual IDBValue& CALL_TYPE Add(const char*, const char*, const char*, const char*);
		// virtual IDBValue& CALL_TYPE Add(const char*, const char*, int);
		// 移除参数
		virtual bool CALL_TYPE Remove(int);
		virtual bool CALL_TYPE Remove(const char*);
		virtual bool CALL_TYPE Clear(void);

    private:
        // 加参数
        template<typename TVAL>
        IDBValue& AddParm(const char*, IDBType::EDBType, TVAL);

    private:
        IActionData& m_act;
        // 类型和值列表
        CDBTypeList m_types;
        CDBValueList m_values;
    };
}
