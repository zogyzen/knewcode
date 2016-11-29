#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // 公共功能活动接口
    class IKCAlgorithmAction
    {
    public:
        // 获取当前时间字符串
		virtual const char* CALL_TYPE GetNowString(void) = 0;
        // 字符串代理
		virtual bool CALL_TYPE SetStringAgent(const char*, const char*) = 0;
		virtual const char* CALL_TYPE GetStringAgent(const char*) = 0;
		// 编码转换
		virtual const char* CALL_TYPE GBK2utf8(const char*) = 0;
		virtual const char* CALL_TYPE utf82GBK(const char*) = 0;
    };

    // 公共功能工厂接口
    class IKCUtilWork
    {
    public:
        // 得到页面对应公共功能活动接口
		virtual IKCAlgorithmAction& CALL_TYPE GetAlgorithmAction(IActionData&) = 0;
    };
}
