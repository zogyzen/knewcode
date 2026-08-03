#pragma once

#include "std.h"

namespace KC
{
    class CBundleContext;
    class CBundle : public IBundleEx
    {
    public:
        CBundle(const char* name, const char* path, const char* aliasList, CBundleContext& ct);
        ~CBundle() override;

        // 得到模块名称（子目录）
        const char* CALL_TYPE getName(void) const override;
        // 得到模块目前状态
        TBundleState CALL_TYPE getState(void) const override;
        // 得到所在目录
        const char* CALL_TYPE getPath(void) const override;
        // 加载DLL
        bool CALL_TYPE load(void) const override;
        // 释放DLL
        bool CALL_TYPE free(void) const override;
        // 启动模块
        bool CALL_TYPE start(void) const override;
        // 停止模块
        bool CALL_TYPE stop(void) const override;
		// 得到模块上下文
        IBundleContext& CALL_TYPE getContext(void) const override;
        // 得到首个服务的别名
        const char* CALL_TYPE getFirstServiceAlias(void) const override;
        // 写日志
        bool CALL_TYPE WriteLog(TLogInfo) const override;

    public:
        // 注册服务
        string registerService(IService& srv, int aliasSort) const;
        bool unregisterService(IService& srv) const;

        // 整个框架启动完成
        bool context_started(void);
        // 整个框架将要停止
        bool context_will_stop(void);

    private:
        // 模块名称（子目录）
        const string m_name;
        // 模块路径
        string m_path;
        // 已注册的服务列表
        mutable std::map<string, IService*> m_mapSrv;
        // 别名列表
        std::vector<string> m_aliases;
        // 插件上下文
        CBundleContext& m_Context;
        // 动态库加载
        mutable dll::shared_library m_lib;
        // 模块生命周期
        mutable IBundleActivator* m_actor = nullptr;
        // 模块状态
        mutable TBundleState m_state;
    };
}
