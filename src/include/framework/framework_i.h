#pragma once

#include "for_user/kc_object_i.h"
#include "framework/bundle_context_i.h"

namespace KC
{
    class IFramework : public IKCObject
	{
	public:
		// 创建、释放插件上下文，参数为模块目录
        virtual IBundleContext* CALL_TYPE NewContext(const char* = "") = 0;
        virtual bool CALL_TYPE FreeContext(IBundleContext*&) = 0;

    protected:
        virtual ~IFramework() = default;
	};

    //constexpr const char c_frameworkDllName[] = "kc_framework";
    constexpr const char c_frameworFuncName[] = "GetFramework";
}
