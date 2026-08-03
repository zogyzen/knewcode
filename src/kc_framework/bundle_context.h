#pragma once

#include "std.h"
#include "kc_lock_work.h"
#include "kc_log_thread.h"

namespace KC
{
    class CFramework;
    class CBundle;
    class CServiceRegistration;
    class CBundleContext : public IBundleContextEx
    {
        friend class CFramework;

    public:
        CBundleContext(IKCStartWork&, CFramework&, string, string);
        ~CBundleContext(void) override;

        // 得到主启动接口
        IKCStartWork& GetMain(void) override;
        // 安装、卸载模块
        IBundle* CALL_TYPE installBundle(const char*, const char*, IBundle::TBundleState, const char* = "") override;
        bool CALL_TYPE uninstallBundle(IBundle*&) override;
        // 获取模块
        bool CALL_TYPE ExistsBundle(const char*) const override;
        const IBundle& CALL_TYPE getBundle(const char*) const override;
        // 注册、注销服务
        IServiceRegistration* CALL_TYPE registerService(IService&, int = 0) override;
        bool CALL_TYPE unregisterService(IServiceRegistration*&) override;
        // 服务的引用、释放
        IServiceReference* CALL_TYPE takeServiceReference(const char*) override;
        bool CALL_TYPE freeServiceReference(IServiceReference*&) override;
        // 得到框架模块目录
        const char* CALL_TYPE getPath(void) const override;
        // 得到网站或应用根路径
        const char* CALL_TYPE getWebsiteRootPath(void) const override;
        // 配置路径转换到本地完整路径
        const char* CALL_TYPE transCfgPathToFullPath(const char*) const override;
        // 得到配置的目录
        const char* CALL_TYPE getPathCfg(const char*) const override;
        // 获取本地化语言选项
        const char* CALL_TYPE getSelLang(void) const override;
        // 获取本地化提示信息
        const char* CALL_TYPE getHint(const char*, const char* = nullptr) const override;
        // 得到配置的日志等级
        int CALL_TYPE GetCfgLogLevel(void) const override;
        // 写日志
        bool CALL_TYPE WriteLog(TLogInfo) const override;
        // 锁
        IKcLockWork& CALL_TYPE LockWork(void) override;
        // 超时时间
        int CALL_TYPE GetTimeOutSeconds(void) const override;
        // 得到服务器编号
        unsigned CALL_TYPE GetSrvID(const char* = "id") const override;
        // 得到系统标志信息
        const char* CALL_TYPE GetSysFlag(const char* = "customer", const char* = nullptr) const override;
        // 得到配置信息
        const char* CALL_TYPE GetCfgInfo(const char*, const char*, const char*) const override;
        // 子配置项
        int CALL_TYPE GetCfgSubCount(const char*) const override;
        bool CALL_TYPE IsCfgSubValid(const char*, int) const override;
        const char* CALL_TYPE GetCfgSubName(const char*, int) const override;
        const char* CALL_TYPE GetCfgSubInfo(const char*, int, const char* = nullptr, const char* = nullptr) const override;
        // 得到配置文件
        const char* CALL_TYPE GetCfgFile(void) const override;
        // 通过别名得到服务标识
        const char* CALL_TYPE GetSrvGUIDByAlias(const char*) const override;
        // 版本信息
        const char* CALL_TYPE VersionInfo(void) const override;
        const char* CALL_TYPE BuildDatetime(void) const override;
        // 加解密
        const char* CALL_TYPE Encrypted(const char*, unsigned long, const char*) const override;
        const char* CALL_TYPE Decrypted(const char*, unsigned long&, const char*) const override;


    public:
        // 配置
        boost::property_tree::ptree& CfgPt(void) override { return m_cfgPt; }

    public:
        // 获取服务的注册
        boost::weak_ptr<IServiceRegistration> getSrvReg(string);

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
        IKCStartWork& m_own;
        // 框架
        CFramework& m_Framework;
        // 模块所在目录
        string m_pathModules;
        // 配置文件
        string m_cfgFile;
        boost::property_tree::ptree m_cfgPt;
        // 多语言选择
        string m_selLang = "en";
        // 信息提示信息
        map<string, string> m_ptHints;
        // 配置的目录列表
        map<string, string> m_CfgDirs;
        // 服务的简化别名
        map<string, string> m_CfgAlias;
        // 超时时间
        int m_timeout_seconds = 7200;
        // 锁管理
        CKcLockWork m_LockWork;
        // 写日志的线程
        mutable CLogThread m_logThrd;
        // 构建时间
        string m_buildDt = "2024.02.06.1300";

    private:
        // 服务层
        boost::shared_mutex m_mtxSrv;
        // 服务注册列表
        typedef boost::shared_ptr<CServiceRegistration> TSrvRegPtr;
        typedef map<string, TSrvRegPtr> TSrvRegMap;
        TSrvRegMap m_SrvRegMap;

    private:
        // 模块层
        boost::mutex m_mtxBnd;
        // 模块列表
        typedef boost::shared_ptr<CBundle> TBundlePtr;
        typedef map<string, TBundlePtr> TBundleMap;
        TBundleMap m_BundleMap;
    };
}
