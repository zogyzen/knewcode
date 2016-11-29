#pragma once

#include "framework/bundle_context_i.h"

namespace KC
{
	class IBundleActivator
	{
	public:
		// 启动模块（注册服务等）
		virtual bool CALL_TYPE start(void) = 0;
		// 停止模块（注销服务等）
		virtual bool CALL_TYPE stop(void) = 0;

    protected:
        virtual CALL_TYPE ~IBundleActivator() = default;
	};

	// 模块初始化和卸载函数
	constexpr const char g_ModuleInitActor[] = "InitActor";
	constexpr const char g_ModuleUninitActor[] = "UninitActor";
}

#define KC_SET_ACTIVATOR(TActor) \
    static TActor* g_actor = nullptr; \
    __extension__ extern "C" \
	{ \
		KC::IBundleActivator& CALL_TYPE InitActor(const KC::IBundle& bundle) \
		{ \
			if (nullptr == g_actor) g_actor = new TActor(bundle); \
			return *g_actor; \
		} \
		void CALL_TYPE UninitActor(void) \
		{ \
			delete g_actor; \
			g_actor = nullptr; \
		} \
	}
