#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // Session接口
    class IKCSession
    {
    public:
		// 设置Session值
		virtual bool CALL_TYPE Set(const char*, const char*) = 0;
		// 获取Session值
		virtual const char* CALL_TYPE Get(const char*) = 0;
		// 移除Session
		virtual bool CALL_TYPE Remove(const char*) = 0;
		// 是否存在Session
		virtual bool CALL_TYPE Exists(const char*) = 0;
    };

    // Session管理
    class IKCSessionWork
    {
    public:
        // 获取Session接口
		virtual IKCSession& CALL_TYPE GetSession(IActionData&) = 0;
    };
}
