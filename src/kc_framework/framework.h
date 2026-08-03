#pragma once

#include "bundle_context.h"

namespace KC
{
    class CFramework : public IFramework
    {
    public:
        CFramework(IKCStartWork& own, string path, string cfg);
        CALL_TYPE ~CFramework() override;

		// 创建、释放插件上下文，参数为配置文件名
        IBundleContext* CALL_TYPE NewContext(const char* = "") override;
        bool CALL_TYPE FreeContext(IBundleContext*&) override;

        // 获取框架路径
        string GetPath() const { return m_path; }

    private:
        IKCStartWork& m_own;
        // 框架所在磁盘目录
        const string m_path;
        // 配置文件
        const string m_cfg;
    };
}
