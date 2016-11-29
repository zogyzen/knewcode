#pragma once

namespace KC
{
    class CISAPIWork
    {
    public:
        CISAPIWork(const char* path);
        ~CISAPIWork();

        // 处理请求
        void Work(EXTENSION_CONTROL_BLOCK& ECB);

    protected:
        // 错误信息
        string m_LibError;
        // 主处理模块动态库句柄
        HINSTANCE m_kc_web_dll_h = NULL;
        // 主框架接口
        IFramework* m_FrameworkIF = nullptr;
        // 主插件上下文
        IBundleContext* m_BundleContextIF = nullptr;
        // 主应用引用
        IServiceReference* m_WebMainRef = nullptr;
    };
}

