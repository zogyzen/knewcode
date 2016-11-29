#include "std.h"
#include "fastcgi_work.h"
#include "request_respond.h"

////////////////////////////////////////////////////////////////////////////////
// CFastCGIWork类
KC::CFastCGIWork::CFastCGIWork(const char* path)
    : m_FxPath(path), m_FxDll(m_FxPath + "/" + c_frameworkDllName), m_framework_dll(m_FxDll.c_str())
{
}

KC::CFastCGIWork::~CFastCGIWork()
{
    if (nullptr != m_BundleContextIF)
        m_BundleContextIF->freeServiceReference(m_WebMainRef);
    if (nullptr != m_FrameworkIF)
        m_FrameworkIF->FreeContext(m_BundleContextIF);
}

// 初始化
void KC::CFastCGIWork::Init(void)
{
    // 初始化锁
    m_mtxInit.lock();
    CAutoRelease _auto(boost::bind(&boost::mutex::unlock, &this->m_mtxInit));
    // 获取主处理接口函数
    if (nullptr == m_FrameworkIF)
    {
        string sLibError = m_framework_dll.LoadLib();
        if (!sLibError.empty())
            throw CKCPageException(0, __FUNCTION__, sLibError + ", in " + __FUNCTION__, "kc_fastcgi_mod", m_FxDll, "");
        typedef IFramework& CALL_TYPE (*FuncGetFramework)(const char*);
        FuncGetFramework _getFramework = m_framework_dll.GetLibFunc<FuncGetFramework>(c_frameworFuncName, sLibError);;
        if (!sLibError.empty())
            throw CKCPageException(1, __FUNCTION__, sLibError + ", in " + __FUNCTION__, "kc_fastcgi_mod", m_FxDll, c_frameworFuncName);
        if (NULL == _getFramework)
            throw CKCPageException(2, __FUNCTION__, string(c_frameworkDllName) + "::" + c_frameworFuncName + " [Can't get DLL function], in" + __FUNCTION__, "kc_fastcgi_mod", m_FxDll, c_frameworFuncName);
        m_FrameworkIF = &_getFramework(m_FxPath.c_str());
        if (nullptr == m_FrameworkIF)
            throw CKCPageException(3, __FUNCTION__, string("Failed to initialize the KC Framework module, in ") + __FUNCTION__, "kc_fastcgi_mod", m_FxDll, c_frameworFuncName);
    }
    // 创建主插件上下文
    if (nullptr == m_BundleContextIF)
    {
        m_BundleContextIF = m_FrameworkIF->NewContext((m_FxPath + "/" + g_ModuleDirectory).c_str());
        if (nullptr == m_BundleContextIF)
            throw CKCPageException(4, __FUNCTION__, string("Failed to create the Module Context, in ") + __FUNCTION__, "kc_fastcgi_mod", m_FxDll, c_frameworFuncName);
    }
    // 获取主处理应用
    if (nullptr == m_WebMainRef)
        m_WebMainRef = m_BundleContextIF->takeServiceReference(c_KCWebWorkSrvGUID);
}

// 获取链接
bool KC::CFastCGIWork::Accept(FCGX_Request& request)
{
    m_mtx.lock();
    CAutoRelease _autoB(boost::bind(&boost::mutex::unlock, &this->m_mtx));
    return FCGX_Accept_r(&request) == 0;
}

// 处理请求
void KC::CFastCGIWork::Work(void)
{
    // 循环等待链接
    FCGX_Request request;
    FCGX_InitRequest(&request, 0, 0);
    while (this->Accept(request))
    {
        // 执行主处理模块
        CWebRequestRespond re(*this, request);
        CAutoRelease _auto(boost::bind(&CWebRequestRespond::CommitResponse, &re));
        try
        {
            if (nullptr == m_WebMainRef) this->Init();
            IKCWebWork& wbwk = dynamic_cast<IServiceReferenceEx*>(m_WebMainRef)->getServiceSafe<IKCWebWork>();
            wbwk.Signal();
            wbwk.RenderPage(re);
        }
        catch(TException& ex)
        {
            string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
            re.AddResponseBody(("Internal server error. (FastCGI " + sPTID + ")<br>" + ex.error_info()).c_str());
        }
        catch(std::exception& ex)
        {
            string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
            re.AddResponseBody(("Internal server error. (FastCGI " + sPTID + ")<br>" + ex.what()).c_str());
        }
        catch(const char* ex)
        {
            string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
            re.AddResponseBody(("Internal server error. (FastCGI " + sPTID + ")<br>" + ex).c_str());
        }
        catch(...)
        {
            string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
            re.AddResponseBody(("Internal server error. (FastCGI " + sPTID + ")<br>Not load page.").c_str());
        }
    }
}
