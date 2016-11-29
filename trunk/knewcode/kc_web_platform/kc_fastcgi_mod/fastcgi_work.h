#pragma once

namespace KC
{
    class CFastCGIWork
    {
    public:
        CFastCGIWork(const char* path);
        ~CFastCGIWork();

        // 初始化
        void Init(void);
        // 处理请求
        void Work(void);

    private:
        // 获取链接
        bool Accept(FCGX_Request&);

    private:
        // 初始化锁
        boost::mutex m_mtxInit;
        // 框架路径、模块
        string m_FxPath, m_FxDll;
        // 主框架动态库
        KLoadLibrary<> m_framework_dll;
        // 主框架接口
        IFramework* m_FrameworkIF = nullptr;
        // 主插件上下文
        IBundleContext* m_BundleContextIF = nullptr;
        // 主应用引用
        IServiceReference* m_WebMainRef = nullptr;
        // 锁
        boost::mutex m_mtx;
    };
}
