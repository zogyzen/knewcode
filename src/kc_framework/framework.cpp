#include "std.h"
#include "framework.h"

// 全局上下文
CBundleContext *g_context = nullptr;

//---------------------------------------------------------------------------
// TBundleContext类
CFramework::CFramework(IKCStartWork& own, string path, string cfg)
    : m_own(own), m_path(path), m_cfg(cfg)
{
    cout << "*[knewcode] CFramework::CFramework: " << m_path << endl << "\t" << m_cfg << endl;
}

CFramework::~CFramework()
{
}

// 创建、释放插件上下文，参数为配置文件名
IBundleContext* CFramework::NewContext(const char* flag)
{
    CBundleContext* result = new CBundleContext(m_own, *this, m_path, m_cfg);
    cout << "*[knewcode] CFramework::NewContext: " << flag << endl;
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
