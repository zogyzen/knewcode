#pragma once

#include "for_user/kc_object_i.h"
#include "framework/bundle_context_i.h"

namespace KC
{
    class IBundleActivator : public IKCObject
	{
	public:
		// 启动模块（注册服务等）
		virtual bool CALL_TYPE start(void) = 0;
        // 停止模块（注销服务等）
        virtual bool CALL_TYPE stop(void) = 0;
        // 整个框架启动完成
        virtual bool CALL_TYPE context_started(void) = 0;
        // 整个框架将要停止
        virtual bool CALL_TYPE context_will_stop(void) = 0;

    protected:
        virtual ~IBundleActivator() = default;
	};

    // 模块初始化和卸载函数
    constexpr const char g_ModuleInitActor[] = "InitActor";
    constexpr const char g_ModuleUninitActor[] = "UninitActor";
}
