#pragma once

#include "std.h"

class CKCSSEWS : public TKCCtrlWork<IKCSSEWS>
{
public:
    using TKCCtrlWork<IKCSSEWS>::TKCCtrlWork;

    // 初始化控制器
    void initAllCtrl(void) override;

    // 发送消息
    bool CALL_TYPE Send(const char* wsid, const char* type, const char* msg, const char* ctrl = nullptr) const override;

protected:
    // 插件配置
    boost::property_tree::ptree m_pt;
};
