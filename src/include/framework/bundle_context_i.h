#pragma once

#include "for_user/page_interface.h"
#include "framework/bundle_i.h"
#include "framework/service_registration_i.h"
#include "framework/service_reference_i.h"

namespace KC
{
    class IBundleContext : public IKCContext
    {
    public:
        // 安装、卸载模块
        virtual IBundle* CALL_TYPE installBundle(const char*, const char*, IBundle::TBundleState, const char* = "") = 0;
        virtual bool CALL_TYPE uninstallBundle(IBundle*&) = 0;
        // 获取模块
        virtual bool CALL_TYPE ExistsBundle(const char*) const = 0;
        virtual const IBundle& CALL_TYPE getBundle(const char*) const = 0;
        // 注册、注销服务
        virtual IServiceRegistration* CALL_TYPE registerService(IService&, int = 0) = 0;
        virtual bool CALL_TYPE unregisterService(IServiceRegistration*&) = 0;
        // 服务的引用、释放
        virtual IServiceReference* CALL_TYPE takeServiceReference(const char*) = 0;
        virtual bool CALL_TYPE freeServiceReference(IServiceReference*&) = 0;

    protected:
        virtual ~IBundleContext(void) = default;
    };

    constexpr const char g_ModuleConfigFile[] = "config.xml";
    constexpr const char c_LogDirectoryName[] = "logs/";
}
