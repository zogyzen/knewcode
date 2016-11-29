#include "std.h"
#include "isapi_work.h"
#include "request_respond.h"

////////////////////////////////////////////////////////////////////////////////
// CISAPIWork类
KC::CISAPIWork::CISAPIWork(const char* path)
    : m_kc_web_dll_h(::LoadLibrary((string(path) + "/" + c_frameworkDllName + ".dll").c_str()))
{
    string strLib = string(path) + "/" + c_frameworkDllName + ".dll";
    if (0 == m_kc_web_dll_h) throw (strLib + " [Can't load DLL file]").c_str();
    // 获取主处理接口函数
    typedef IFramework& CALL_TYPE (*FuncGetFramework)(const char*);
    FuncGetFramework _getFramework = (FuncGetFramework)::GetProcAddress(m_kc_web_dll_h, c_frameworFuncName);
    if (0 == _getFramework)
        throw (strLib + "::" + c_frameworFuncName + " [Can't get DLL function]").c_str();
    m_FrameworkIF = &_getFramework(path);
    if (nullptr == m_FrameworkIF) throw "Failed to initialize the KC Framework module..";
    // 创建主插件上下文
    m_BundleContextIF = m_FrameworkIF->NewContext((string(path) + "/" + g_ModuleDirectory).c_str());
    if (nullptr == m_BundleContextIF) throw "Failed to create the Module Context..";
}

KC::CISAPIWork::~CISAPIWork()
{
    if (nullptr != m_BundleContextIF)
        m_BundleContextIF->freeServiceReference(m_WebMainRef);
    if (nullptr != m_FrameworkIF)
        m_FrameworkIF->FreeContext(m_BundleContextIF);
    // 卸载动态库
    ::FreeLibrary(m_kc_web_dll_h);
}

void KC::CISAPIWork::Work(EXTENSION_CONTROL_BLOCK& ECB)
{
    // 执行主处理模块
    CKCRequestRespond re(*this, ECB);
    try
    {
        if (nullptr == m_WebMainRef)
            m_WebMainRef = m_BundleContextIF->takeServiceReference(c_KCWebWorkSrvGUID);
        m_WebMainRef->getServiceSafe<IKCWebWork>().RenderPage(re);
    }
    catch(std::exception& ex)
    {
        throw ex.what();
    }
    catch(...)
    {
        throw "Not load page.";
    }
}
