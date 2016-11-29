#pragma once

#include "../inner_var.h"

namespace KC { namespace one_level
{
    // page����
    class TInvarPage : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarPage(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarPage() = default;

        // �õ�����
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // �õ��ڲ�����ֵ
        bool Exists(boost::any);
        // �õ��ڲ�����ֵ
        boost::any GetValue(IActionData&, boost::any);
        // �����ڲ�����ֵ
        void SetValue(boost::any, boost::any);

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;
    };
}}
