#include "std.h"
#include "framework.h"

// 全局上下文
CBundleContext *g_context = nullptr;

//---------------------------------------------------------------------------
// TBundleContext类
CFramework::CFramework(string path) : m_path(path)
{
}

CFramework::~CFramework()
{
}

// 创建、释放插件上下文，参数为配置文件名
IBundleContext* CFramework::NewContext(const char* pathModules)
{
    CBundleContext* result = new CBundleContext(*this, CUtilFunc::FormatPath(pathModules).c_str());
    try
    {
        result->startup();
        if (nullptr == g_context) g_context = result;
    }
    catch(...)
    {
        delete result;
        result = nullptr;
        throw;
    }
    return result;
}

bool CFramework::FreeContext(IBundleContext*& iCont)
{
    if (nullptr != iCont)
    {
        CBundleContext* pCont = dynamic_cast<CBundleContext*>(iCont);
        if (nullptr != pCont) pCont->shutoff();
        if (iCont == g_context) g_context = nullptr;
        delete pCont;
        iCont = nullptr;
    }
    return nullptr == iCont;
}

// 获取框架路径
string CFramework::GetPath() const
{
    return m_path;
}
