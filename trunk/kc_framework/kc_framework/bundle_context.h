#pragma once

#include "service_registration.h"
#include "service_reference.h"
#include "bundle.h"

#include "kc_lock_work.h"
#include "kc_log_thread.h"

namespace KC
{
    class CFramework;
    class CBundleContext : public IBundleContextEx
    {
        friend class CFramework;

    public:
        CBundleContext(CFramework&, const char*);
        virtual CALL_TYPE ~CBundleContext(void);

        // 安装、卸载模块
        IBundle* CALL_TYPE installBundle(const char*, const char*, IBundle::TBundleState) override;
        bool CALL_TYPE uninstallBundle(IBundle*&) override;
        // 获取模块
        bool CALL_TYPE ExistsBundle(const char*) const override;
        const IBundle& CALL_TYPE getBundle(const char*) const override;
        // 注册、注销服务
        IServiceRegistration* CALL_TYPE registerService(IService&) override;
        bool CALL_TYPE unregisterService(IServiceRegistration*&) override;
        // 服务的引用、释放
        IServiceReference* CALL_TYPE takeServiceReference(const char*) override;
        bool CALL_TYPE freeServiceReference(IServiceReference*&) override;
        // 得到模块目录
        const char* CALL_TYPE getPath(void) const override;
        // 得到配置的目录
        const char* CALL_TYPE getPathCfg(const char*) const override;
        // 获取本地化提示信息
        const char* CALL_TYPE getHint(const char*) const override;
        // 得到配置的日志等级
        int CALL_TYPE GetCfgLogLevel(void) const override;
        // 写日志
        bool CALL_TYPE WriteLog(TLogInfo) const override;
        // 信号
        void CALL_TYPE Signal(void) override;
        ISignoCatch& CALL_TYPE SignoCatch(void) override;
        // 锁
        IKcLockWork& CALL_TYPE LockWork(void) override;
        // 超时时间
        int CALL_TYPE GetTimeOutSeconds(void) const override;

    protected:
        // 注销服务
        bool unregisterService(const char*);

    private:
        // 启动上下文
        void startup(void);
        // 关闭上下文
        void shutoff(void);
        // 初始化日志
        void init_log(void);
        // 初始化信息提示信息
        void init_hint(void);
        // 初始化超时时间
        void init_timeout(void);
        // 获取绝对路径
        string GetFullPath(string);

    private:
        // 框架
        CFramework& m_Framework;
        // 模块所在目录
        string m_pathModules;
        // 配置文件
        string m_cfgFile;
        // 配置的日志等级
        log::trivial::severity_level m_logLevel = log::trivial::warning;
        // 信息提示信息
        map<string, string> m_ptHints;
        // 配置的目录列表
        map<string, string> m_CfgDirs;
        // 超时时间
        int m_timeout_seconds = 7200;
        // 锁管理
        CKcLockWork m_LockWork;
        // 写日志的线程
        mutable CLogThread m_logThrd;

    private:
        // 服务层
        CKcMutex m_mtxSrv;
        // 服务注册列表
        typedef std::shared_ptr<CServiceRegistration> TSrvRegPtr;
        typedef map<string, TSrvRegPtr> TSrvRegMap;
        TSrvRegMap m_SrvRegMap;
        // 服务引用列表
        typedef map<long, CServiceReference*> TSrvRefLst;        typedef map<string, TSrvRefLst> TSrvRefMap;
        TSrvRefMap m_SrvRefMap;

    private:
        // 模块层
        CKcMutex m_mtxBundle;
        // 模块列表        typedef std::shared_ptr<CBundle> TBundlePtr;        typedef map<string, TBundlePtr> TBundleMap;
        TBundleMap m_BundleMap;
    };
}
