#pragma once

#include "framework/bundle_context_i.h"

namespace KC
{
	class IFramework
	{
	public:
		// 创建、释放插件上下文，参数为模块目录
		virtual IBundleContext* CALL_TYPE NewContext(const char*) = 0;
        virtual bool CALL_TYPE FreeContext(IBundleContext*&) = 0;

    protected:
        virtual CALL_TYPE ~IFramework() = default;
	};

    constexpr const char c_frameworkDllName[] = "kc_framework";
    constexpr const char c_frameworFuncName[] = "GetFramework";
}
