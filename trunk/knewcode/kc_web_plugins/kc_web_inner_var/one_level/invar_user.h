#pragma once

#include "../inner_var.h"

namespace KC { namespace one_level
{
    // user����
    class TInvarUser : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarUser(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarUser() = default;

        // �õ�����
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // �õ��ڲ�����ֵ
        bool Exists(boost::any);
        // �õ��ڲ�����ֵ
        boost::any GetValue(boost::any);
        // �����ڲ�����ֵ
        void SetValue(boost::any, boost::any);

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;

    private:
        // user�ڲ�����
        struct TInnerVarKey
        {
            // ��ֵ
            boost::any key;
            // ���졢����
            TInnerVarKey(boost::any k);
            TInnerVarKey(const TInnerVarKey& clone);
            TInnerVarKey operator=(const TInnerVarKey& rv);
            // �Ƚ�
            bool operator<(const TInnerVarKey& rv) const;
        };
        typedef map<TInnerVarKey, boost::any> TUserInVarList;
        TUserInVarList m_UserInVarList;

    private:
        // ���user�ڲ�����
        void SetUserInVar(boost::any, boost::any);
        // ��ȡuser�ڲ�����
        boost::any GetUserInVar(boost::any);
    };
}}
