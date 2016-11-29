#pragma once

namespace KC
{
    class CWebWork
    {
    public:
        CWebWork(const char* path);
        ~CWebWork();

        // 处理请求
        string Work(const char* request);

    private:
        // 主目录
        const string m_path;
        // 错误信息
        string m_LibError;
        // 主框架动态库
        KLoadLibrary<true> m_framework_dll;
        // 主框架接口
        IFramework* m_FrameworkIF = nullptr;
        // 主插件上下文
        IBundleContext* m_BundleContextIF = nullptr;
        // 主应用引用
        IServiceReference* m_WebMainRef = nullptr;
    };
}
