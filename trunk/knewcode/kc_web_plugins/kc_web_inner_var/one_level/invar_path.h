#pragma once

#include "../inner_var.h"

namespace KC { namespace one_level
{
    // page����
    class TInvarPath : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarPath(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarPath() = default;

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
    };
}}
