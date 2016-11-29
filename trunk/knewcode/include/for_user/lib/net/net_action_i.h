#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // 网络功能活动接口
    class IKCNetAction
    {
    public:
        // 获取网页
		virtual const char* CALL_TYPE SyncHttpGet(const char*, const char*, int = 80) = 0;
    };

    // 网络功能工厂接口
    class IKCNetWork
    {
    public:
        // 得到页面对应网络功能活动接口
		virtual IKCNetAction& CALL_TYPE GetNetAction(IActionData&) = 0;
    };
}
