#pragma once

#include "kc_web/web_struct.h"
#include "kc_web/page_data_i.h"

namespace KC
{
    // ÐÅºÅ²¶×½ÊµÀý
    template<typename TF>
    class TSignoTry : public ISignoCatch::ISignoTry
    {
    public:
        TSignoTry(TF& t, IKCRequestRespond& re, const char* s = "@") : m_tf(t), m_re(re), m_info(s) {}

        virtual const char* CALL_TYPE Name(void) { return typeid(m_tf).name(); }
        virtual const char* CALL_TYPE Info(void) { return m_info.c_str(); }
        virtual const char* CALL_TYPE Detail(int& id)
        {
            id = 0;
            m_SignoCurrPosInfo = "";
            IActionData* pAct = this->m_re.GetActionData();
            if (nullptr != pAct)
            {
                IKCActionData& act = dynamic_cast<IKCActionData&>(pAct->GetActNow());
                id = act.GetCurrLineID();
                this->m_SignoCurrPosInfo = act.GetCurrPosInfo();
            }
            return m_SignoCurrPosInfo.c_str();
        }
        virtual void CALL_TYPE Try(void) { m_tf(); }

    private:
        TF& m_tf;
        IKCRequestRespond& m_re;
        string m_info;
        string m_SignoCurrPosInfo;
    };
}
