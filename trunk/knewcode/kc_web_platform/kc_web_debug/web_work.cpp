#include "std.h"
#include "web_work.h"
#include "request_respond.h"

////////////////////////////////////////////////////////////////////////////////
// CISAPIWork类
KC::CWebWork::CWebWork(const char* path)
    : m_path(path)
    , m_framework_dll((string(path) + "/" + c_frameworkDllName).c_str(), m_LibError)
{
    if (!m_LibError.empty()) throw m_LibError.c_str();
    // 获取主处理接口函数
    typedef IFramework& CALL_TYPE (*FuncGetFramework)(const char*);
    FuncGetFramework _getFramework = m_framework_dll.GetLibFunc<FuncGetFramework>(c_frameworFuncName, m_LibError);;
    if (!m_LibError.empty()) throw m_LibError.c_str();
    if (NULL == _getFramework)
        throw (string(c_frameworkDllName) + "::" + c_frameworFuncName + " [Can't get DLL function]").c_str();
    m_FrameworkIF = &_getFramework(path);
    if (nullptr == m_FrameworkIF)
        throw "Failed to initialize the KC Framework module.";
    // 创建主插件上下文
    m_BundleContextIF = m_FrameworkIF->NewContext((string(path) + "/" + g_ModuleDirectory).c_str());
    if (nullptr == m_BundleContextIF)
        throw "Failed to create the Module Context.";
}

KC::CWebWork::~CWebWork()
{
    if (nullptr != m_BundleContextIF)
        m_BundleContextIF->freeServiceReference(m_WebMainRef);
    if (nullptr != m_FrameworkIF)
        m_FrameworkIF->FreeContext(m_BundleContextIF);
}

// 处理请求
string KC::CWebWork::Work(const char* request)
{
    try
    {
        if (nullptr == m_WebMainRef)
            m_WebMainRef = m_BundleContextIF->takeServiceReference(c_KCWebWorkSrvGUID);
        IKCWebWork& wbwk = dynamic_cast<IServiceReferenceEx*>(m_WebMainRef)->getServiceSafe<IKCWebWork>();
        wbwk.Signal();
        CWebRequestRespond re(*this, request, m_path.c_str());
        wbwk.RenderPage(re);
        return re.m_response;
    }
    catch(std::exception& ex)
    {
        throw ex.what();
    }
    catch(const char* ex)
    {
        throw;
    }
    catch(...)
    {
        throw "Not load page.";
    }
}
