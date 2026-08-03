#pragma once

#include "std.h"

class CCtrlOSExec : public IKCController
{
public:
    CCtrlOSExec(IKCOSExec& own, string sName, property_tree::ptree& pt);
    ~CCtrlOSExec(void) override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

private:
    // 宿主
    IKCOSExec& m_own;
    // 控制器名称
    string m_name;
};
