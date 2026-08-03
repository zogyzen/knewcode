#pragma once

#include "std.h"

class CKCOSFile : public TKCCtrlWork<IKOSFile>
{
public:
    using TKCCtrlWork<IKOSFile>::TKCCtrlWork;

    // 初始化控制器
    void initAllCtrl(void) override;

protected:
    // 插件配置
    boost::property_tree::ptree m_pt;
};
