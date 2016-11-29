#pragma once

#include "../inner_var.h"

namespace KC { namespace one_level
{
    // page变量
    class TInvarPage : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarPage(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarPage() = default;

        // 得到名称
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // 得到内部变量值
        bool Exists(boost::any);
        // 得到内部变量值
        boost::any GetValue(IActionData&, boost::any);
        // 设置内部变量值
        void SetValue(boost::any, boost::any);

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;
    };
}}
