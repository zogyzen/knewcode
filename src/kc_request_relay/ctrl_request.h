#pragma once

#include "std.h"

// “转发”控制器
class CCtrlRelay : public IKCController
{
public:
    CCtrlRelay(IKCRelayREST& own, std::string sName, boost::property_tree::ptree&);
    ~CCtrlRelay() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

private:
    // 宿主
    IKCRelayREST& m_own;
    string m_name;
    // web request 调用库
    TSubModule<ISLWebRequestEx> m_requestLib;
};
