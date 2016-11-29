#pragma once

#include "framework/bundle_i.h"
#include "framework/service_registration_i.h"
#include "framework/service_reference_i.h"

namespace KC
{
	class IBundleContext
	{
	public:
		// 安装、卸载模块
		virtual IBundle* CALL_TYPE installBundle(const char*, const char*, IBundle::TBundleState) = 0;
		virtual bool CALL_TYPE uninstallBundle(IBundle*&) = 0;
		// 获取模块
        virtual bool CALL_TYPE ExistsBundle(const char*) const = 0;
        virtual const IBundle& CALL_TYPE getBundle(const char*) const = 0;
		// 注册、注销服务
		virtual IServiceRegistration* CALL_TYPE registerService(IService&) = 0;
		virtual bool CALL_TYPE unregisterService(IServiceRegistration*&) = 0;
		// 服务的引用、释放
		virtual IServiceReference* CALL_TYPE takeServiceReference(const char*) = 0;
        virtual bool CALL_TYPE freeServiceReference(IServiceReference*&) = 0;
        // 得到模块目录
        virtual const char* CALL_TYPE getPath(void) const = 0;
        // 得到配置的目录
        virtual const char* CALL_TYPE getPathCfg(const char*) const = 0;
        // 获取本地化提示信息
        virtual const char* CALL_TYPE getHint(const char*) const = 0;
        // 得到配置的日志等级
        virtual int CALL_TYPE GetCfgLogLevel(void) const = 0;
        // 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogDebug(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogInfo(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogFatal(const char*, const char*, const char* = "") const = 0;
        // 信号
        virtual void CALL_TYPE Signal(void) = 0;
        virtual ISignoCatch& CALL_TYPE SignoCatch(void) = 0;
        // 锁
        virtual IKcLockWork& CALL_TYPE LockWork(void) = 0;
        // 超时时间
        virtual int CALL_TYPE GetTimeOutSeconds(void) const = 0;

    protected:
        virtual CALL_TYPE ~IBundleContext(void) = default;
	};

	constexpr const char g_ModuleDirectory[] = "";
	constexpr const char g_ModuleConfigFile[] = "config.xml";
    constexpr const char c_LogDirectoryName[] = "logs/";
}
