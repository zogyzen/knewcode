#pragma once

namespace KC
{
    class CDBParms : public IDBParms
    {
    public:
        CDBParms(IActionData&);
        virtual CALL_TYPE ~CDBParms() = default;

        // ���������б�
		virtual IDBTypeList& CALL_TYPE DBTypeList(void);
		// �õ�ֵ
		virtual IDBValue& CALL_TYPE Value(const char*);
		virtual IDBValue& CALL_TYPE Value(int);
        // ��Ӳ���
		virtual IDBValue& CALL_TYPE Add(const char*, int);
		virtual IDBValue& CALL_TYPE Add(const char*, double);
		virtual IDBValue& CALL_TYPE Add(const char*, const char*);
		virtual IDBValue& CALL_TYPE Add(const char*, const char*, const char*, const char*);
		// virtual IDBValue& CALL_TYPE Add(const char*, const char*, int);
		// �Ƴ�����
		virtual bool CALL_TYPE Remove(int);
		virtual bool CALL_TYPE Remove(const char*);
		virtual bool CALL_TYPE Clear(void);

    private:
        // �Ӳ���
        template<typename TVAL>
        IDBValue& AddParm(const char*, IDBType::EDBType, TVAL);

    private:
        IActionData& m_act;
        // ���ͺ�ֵ�б�
        CDBTypeList m_types;
        CDBValueList m_values;
    };
}
