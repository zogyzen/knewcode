#pragma once

#include "std.h"

namespace KC
{
    class CCtrlExcel : public IKCController
    {
    public:
        CCtrlExcel(string sName);
        ~CCtrlExcel() override;

        // 执行控制器
        void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

    private:
        string m_name;
        string m_server;
    };
}
