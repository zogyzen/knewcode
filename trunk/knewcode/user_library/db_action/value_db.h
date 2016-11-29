#pragma once

namespace KC
{
    // 数据值
    class CDBValue : public IDBValue
    {
    public:
        CDBValue(IActionData&, IDBType&, IDBValueList&);
        CDBValue(IActionData&, IDBType&, IDBValueList&, boost::any);
        CDBValue(IActionData&, IDBType&, IDBValueList&, string);
        virtual CALL_TYPE ~CDBValue() = default;

        // 数据类型
		virtual IDBType& CALL_TYPE DBType(void);
		// 值
		virtual bool CALL_TYPE IsNULL(void) const;
		virtual void CALL_TYPE SetNULL(void);
		virtual int CALL_TYPE AsInt(void) const;
		virtual void CALL_TYPE SetInt(int);
		virtual double CALL_TYPE AsDouble(void) const;
		virtual void CALL_TYPE SetDouble(double);
		virtual const char* CALL_TYPE AsString(void) const;
		virtual void CALL_TYPE SetString(const char*);
		virtual const char* CALL_TYPE AsString(const char*, const char*) const;
		virtual void CALL_TYPE SetString(const char*, const char*, const char*);
		// virtual const char* CALL_TYPE AsBytes(int&) const;
		// virtual void CALL_TYPE SetBytes(const char*, int);
        // 数据值列表
		virtual IDBValueList& CALL_TYPE DBValueList(void);

    public:
        // 创建缓冲区
        static string MakeBytes(const char*, int = 0);
        // 设置获取值
		boost::any CALL_TYPE AsAny(void) const;
		void CALL_TYPE SetAny(boost::any);
		// 时间类型转换
		string TimeToStr(const std::tm&) const;
		std::tm StrToTime(const string&) const;
		int TimeToInt(const std::tm&) const;
		std::tm IntToTime(int) const;

    private:
        IActionData& m_act;
        IDBType& m_type;
        IDBValueList& m_list;
        bool m_IsNULL = true;
        boost::any m_value;
        mutable string m_valueStr;
    };
    typedef boost::shared_ptr<CDBValue> TDBValuePtr;

    // 数据值列表
    class CDBValueList : public IDBValueList
    {
    public:
        CDBValueList(IActionData&, IDBTypeList&);
        virtual CALL_TYPE ~CDBValueList() = default;

        // 数据类型列表
		virtual IDBTypeList& CALL_TYPE DBTypeList(void);
		// 得到值
		virtual IDBValue& CALL_TYPE Value(int);
		virtual IDBValue& CALL_TYPE Value(const char*);

    public:
        // 加值
        void Add(TDBValuePtr);
        // 删值
        void Del(int);
        void Clear(void);

    private:
        IActionData& m_act;
        IDBTypeList& m_types;
        // 值列表
        vector<TDBValuePtr> m_DBValueList;
    };
}
