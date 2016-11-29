#include "std.h"
#include "apache_work.h"
#include "request_respond.h"

////////////////////////////////////////////////////////////////////////////////
// CAPACHEWork类
KC::CAPACHEWork::CAPACHEWork(string fxPath, string pgPath)
    : m_FxPath(fxPath), m_FxDll(m_FxPath + "/" + c_frameworkDllName), m_PgPath(pgPath), m_framework_dll(m_FxDll.c_str())
{
}

KC::CAPACHEWork::~CAPACHEWork()
{
    if (nullptr != m_BundleContextIF)
        m_BundleContextIF->freeServiceReference(m_WebMainRef);
    if (nullptr != m_FrameworkIF)
        m_FrameworkIF->FreeContext(m_BundleContextIF);
}

// 初始化
void KC::CAPACHEWork::Init(void)
{
    // 初始化锁
    m_mtxInit.lock();
    CAutoRelease _auto(boost::bind(&boost::mutex::unlock, &this->m_mtxInit));
    // 获取主处理接口函数
    if (nullptr == m_FrameworkIF)
    {
        string sLibError = m_framework_dll.LoadLib();
        if (!sLibError.empty())
            throw CKCPageException(0, __FUNCTION__, sLibError + ", in " + __FUNCTION__, "kc_apache_mod", m_FxDll, "");
        typedef IFramework& CALL_TYPE (*FuncGetFramework)(const char*);
        FuncGetFramework _getFramework = m_framework_dll.GetLibFunc<FuncGetFramework>(c_frameworFuncName, sLibError);;
        if (!sLibError.empty())
            throw CKCPageException(1, __FUNCTION__, sLibError + ", in " + __FUNCTION__, "kc_apache_mod", m_FxDll, c_frameworFuncName);
        if (NULL == _getFramework)
            throw CKCPageException(2, __FUNCTION__, string(c_frameworkDllName) + "::" + c_frameworFuncName + " [Can't get DLL function], in" + __FUNCTION__, "kc_apache_mod", m_FxDll, c_frameworFuncName);
        m_FrameworkIF = &_getFramework(m_FxPath.c_str());
        if (nullptr == m_FrameworkIF)
            throw CKCPageException(3, __FUNCTION__, string("Failed to initialize the KC Framework module, in ") + __FUNCTION__, "kc_apache_mod", m_FxDll, c_frameworFuncName);
    }
    // 创建主插件上下文
    if (nullptr == m_BundleContextIF)
    {
        m_BundleContextIF = m_FrameworkIF->NewContext((m_FxPath + "/" + g_ModuleDirectory).c_str());
        if (nullptr == m_BundleContextIF)
            throw CKCPageException(4, __FUNCTION__, string("Failed to create the Module Context, in ") + __FUNCTION__, "kc_apache_mod", m_FxDll, c_frameworFuncName);
    }
    // 获取主处理应用
    if (nullptr == m_WebMainRef)
        m_WebMainRef = m_BundleContextIF->takeServiceReference(c_KCWebWorkSrvGUID);
}

// 处理请求
void KC::CAPACHEWork::Work(request_rec& r)
{
    try
    {
        // 设置返回类型
        r.content_type = "text/html";
        //apr_table_set(r.headers_out, "Set-Cookie", "KC_ID=101; path=/");
        // 初始化主模块接口
        if (nullptr == m_WebMainRef) this->Init();
        IKCWebWork& wbwk = dynamic_cast<IServiceReferenceEx*>(m_WebMainRef)->getServiceSafe<IKCWebWork>();
        // 注册异常处理信号
        wbwk.Signal();
        // 执行页面请求
        CWebRequestRespond re(*this, r, m_PgPath);
        wbwk.RenderPage(re);
    }
    catch(TException& ex)
    {
        string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
        ap_rputs(("Internal server error. (Apache " + sPTID + ")<br>" + ex.error_info()).c_str(), &r);
    }
    catch(std::exception& ex)
    {
        string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
        ap_rputs(("Internal server error. (Apache " + sPTID + ")<br>" + ex.what()).c_str(), &r);
    }
    catch(const char* ex)
    {
        string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
        ap_rputs(("Internal server error. (Apache " + sPTID + ")<br>" + ex).c_str(), &r);
    }
    catch(...)
    {
        string sPTID = lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self());
        ap_rputs(("Internal server error. (Apache " + sPTID + ")<br>Not load page.").c_str(), &r);
    }
}
