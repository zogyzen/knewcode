#pragma once

#include "std.h"

namespace KC
{
    class CCtrlHelloWorld : public IKCController
    {
    public:
        CCtrlHelloWorld(string sName);
        ~CCtrlHelloWorld() override = default;

        // 执行控制器
        void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

    private:
        string m_name;
    };
}
