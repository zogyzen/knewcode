#pragma once

#include "bundle_context.h"

namespace KC
{
    class CFramework : public IFramework
    {
    public:
        CFramework(string path);
        virtual CALL_TYPE ~CFramework();

		// 创建、释放插件上下文，参数为配置文件名
		virtual IBundleContext* CALL_TYPE NewContext(const char*);
		virtual bool CALL_TYPE FreeContext(IBundleContext*&);

        // 获取框架路径
        string GetPath() const;

    private:
        // 框架所在磁盘目录
        const string m_path;
    };
}
