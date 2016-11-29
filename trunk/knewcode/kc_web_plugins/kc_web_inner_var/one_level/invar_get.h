#pragma once

#include "../inner_var.h"

namespace KC { namespace one_level
{
    // get����
    class TInvarGet : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarGet(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarGet() = default;

        // �õ�����
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // �õ��ڲ�����ֵ
        bool Exists(boost::any);
        // �õ��ڲ�����ֵ
        boost::any GetValue(boost::any);
        // �����ڲ�����ֵ
        void SetValue(boost::any, boost::any);

    public:
        // �����б�����
        template<typename ValT>
        using ArgValListT = map<string, ValT>;
        // ��ʼ������
        template<typename ValT>
        static void InitParms(ArgValListT<ValT>& arg, string str, string splt = "&")
        {
            // todo: str��Ҫ����ת��
            vector<string> vecArg;
            boost::split(vecArg, str, boost::is_any_of(splt));
            for (string s: vecArg)
            {
                string sOne = boost::trim_copy(s);
                vector<string> vecVal;
                boost::split(vecVal, sOne, boost::is_any_of("="));
                if (vecVal.size() > 1)
                    arg.insert(make_pair(vecVal[0], ValT(vecVal[0], vecVal[1])));
            }
        }
        // ��ȡ����ֵ
        template<typename ValT>
        static string GetStringValue(ArgValListT<ValT>& arg, boost::any arr)
        {
            string sArr = TInvarGet::GetArrIndex(arr), sResult = "";
            // ��ȡ����ֵ
            auto itVal = arg.find(sArr);
            if (arg.end() != itVal)
                sResult = itVal->second.val;
            return sResult;
        }
        // ��ȡ�±�
        static string GetArrIndex(boost::any);

    private:
        // ����ֵ����
        struct ArgValueT
        {
            string name, val;
            // ���졢����
            ArgValueT(string n, string v) : name(n), val(v) {}
            ArgValueT(const ArgValueT& c) : name(c.name), val(c.val) {}
            ArgValueT& operator=(const ArgValueT& c) { name = c.name; val = c.val; return *this; }
        };

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;
        // ������
        ArgValListT<ArgValueT> m_mapArg;
    };
}}
