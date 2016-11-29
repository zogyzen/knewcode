#pragma once

namespace KC
{
    class CBundleContext;
    class CBundle : public IBundleEx
    {
    public:
        CBundle(const char* name, const char* path, CBundleContext& ct);
        virtual CALL_TYPE ~CBundle();

        // 得到模块名称（子目录）
        virtual const char* CALL_TYPE getName(void) const;
        // 得到模块目前状态
        virtual TBundleState CALL_TYPE getState(void) const;
        // 得到所在目录
        virtual const char* CALL_TYPE getPath(void) const;
        // 加载DLL
        virtual bool CALL_TYPE load(void) const;
        // 释放DLL
        virtual bool CALL_TYPE free(void) const;
        // 启动模块
        virtual bool CALL_TYPE start(void) const;
        // 停止模块
        virtual bool CALL_TYPE stop(void) const;
		// 得到模块上下文
		virtual IBundleContext& CALL_TYPE getContext(void) const;
        // 写日志
        virtual bool CALL_TYPE WriteLog(TLogInfo) const;

    private:
        // 模块名称（子目录）
        const string m_name;
        // 模块路径
        string m_path;
        // 插件上下文
        CBundleContext& m_Context;
        // 动态库加载
        mutable KLoadLibrary<true>* m_libPtr = nullptr;
        // 模块生命周期
        mutable IBundleActivator* m_actor = nullptr;
        // 模块状态
        mutable TBundleState m_state;
    };
}
