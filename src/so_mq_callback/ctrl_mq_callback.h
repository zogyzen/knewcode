#pragma once

#include "std.h"

class CCtrlWebMQCallback : public IKCController
{
public:
    CCtrlWebMQCallback(string sDir, string sName);
    ~CCtrlWebMQCallback() override = default;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

private:
    string m_dir, m_name;
};
