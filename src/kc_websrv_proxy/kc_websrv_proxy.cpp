#include "kc_websrv_proxy.h"
#include "request_respond.h"
#include "websrv_conn.h"

std::shared_ptr<KCWebSrvProxy> g_wsProxy;

extern "C"
{
    // 获取代理接口
    IKCWebSrvProxy& CALL_TYPE makeProxy(IWSProxyServerCB& ps)
    {
        cout << "*[knewcode] makeProxy: " << ps.fxPath() << endl;
        // ExceptBacktrace::SetExceptFilter(ps.fxPath());
        if (g_wsProxy.get() == nullptr) g_wsProxy.reset(new KCWebSrvProxy(ps));
        return *g_wsProxy;
    }

    // 释放代理接口
    void CALL_TYPE releaseProxy(void)
    {
        g_wsProxy.reset();
        cout << "*[knewcode] releaseProxy." << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
// KCWebSrvProxy 类
KCWebSrvProxy::KCWebSrvProxy(IWSProxyServerCB& srv)
    : m_running(true), m_srvCB(srv)
    , m_contHelp(*this, srv.fxPath(), srv.cfgFile()), m_BundleContextIF(m_contHelp.getContext())
    , m_WebMainRef(dynamic_cast<IServiceReferenceEx&>(*m_BundleContextIF.takeServiceReference(c_KCWebWorkSrvGUID)))
    , m_WebApiWrkRef(dynamic_cast<IServiceReferenceEx&>(*m_BundleContextIF.takeServiceReference(c_KCWebApiWorkSrvGUID)))
{
    cout << "*[knewcode] KCWebSrvProxy::KCWebSrvProxy." << endl;
}

KCWebSrvProxy::~KCWebSrvProxy()
{
    m_running = false;
    // 释放链接
    {
        boost::unique_lock<boost::shared_mutex> lck(m_mtxWC);
        m_wcs.clear();
    }
    // 释放框架
    IServiceReference* webApiWrk = &m_WebApiWrkRef;
    m_BundleContextIF.freeServiceReference(webApiWrk);
    IServiceReference* webMain = &m_WebMainRef;
    m_BundleContextIF.freeServiceReference(webMain);
}

// 初始化
void KCWebSrvProxy::Init(void)
{
    string sLog = "VirtualPath\n";
    cout << "*[knewcode] KCWebSrvProxy::Init: VirtualPath" << endl;
    // 虚拟目录
    string sVPth = m_srvCB.VirtualPath();
    vector<string> vctItm;
    boost::algorithm::split(vctItm, sVPth, boost::is_any_of("\n"));
    for (auto itm : vctItm)
    {
        vector<string> vctVP;
        boost::algorithm::split(vctVP, itm, boost::is_any_of("\t"));
        string sUri = boost::algorithm::trim_copy(vctVP[0]);
        if ("/" != sUri) sUri = boost::trim_right_copy_if(sUri, boost::is_any_of("/\\"));
        if (vctVP.size() == 2 && !sUri.empty())
        {
            string sLocal = boost::algorithm::trim_copy(vctVP[1]);
            if ("/" != sLocal) sLocal = boost::trim_right_copy_if(sLocal, boost::is_any_of("/\\"));
            string sLocalFull = boost::filesystem::path(sLocal).is_absolute() ? sLocal
                                : CUtilFunc::ToAbsPath(sLocal, CUtilFunc::PCharSafeToStr(GetContext().GetMain().GetApPath()));
            auto it = m_vPath.find(sUri);
            if (m_vPath.end() == it && !sLocal.empty())
            {
                string sLine = (boost::format("\t%-20s\t%s (%S)\n") % sUri % sLocalFull % sLocal).str();
                cout << sLine;
                sLog += sLine;
                m_vPath.insert(make_pair(sUri, sLocalFull));
                m_vPathVct.push_back(sUri);
            }
        }
    }
    m_WebMainRef.WriteLogTrace(sLog.c_str(), __CURR_CODE_PLACE_C__);
    // 启动成功日志
    m_WebMainRef.WriteLogInfo(m_BundleContextIF.getHint("Start_framework_success"), __CURR_CODE_PLACE_C__);
    // 初始化系统
    m_WebApiWrkRef.getServiceSafe<IKCWebApiWork>().InitSys();
}
void KCWebSrvProxy::Free(void)
{
    // 退出日志
    m_WebMainRef.WriteLogInfo(((boost::format(m_BundleContextIF.getHint("End_framework")) % "Begin" % "").str()).c_str(), __CURR_CODE_PLACE_C__);
    // 卸载系统
    m_vPathVct.clear();
    m_vPath.clear();
    m_WebApiWrkRef.getServiceSafe<IKCWebApiWork>().FreeSys();
}

// 处理请求
void KCWebSrvProxy::Work(IWSProxyRequestCB& r)
{
    auto fRespondErr = [&](string sMsg, string sType)
    {
        string sPTID = std::to_string(boost::this_process::get_id()) + ":" + boost::lexical_cast<string>(boost::this_thread::get_id());
        string sErrMsg = "Internal server error. (" + sPTID + " - " + sType + ") " + sMsg;
        r.AddResponseBody(CCtrlCommon::GetRespondJson(22, sErrMsg));
    };
    try
    {
        // 主模块接口
        IKCWebWork& wbwk = m_WebMainRef.getServiceSafe<IKCWebWork>();
        // 执行页面请求
        CWebRequestRespond re(*this, r, m_srvCB);
        r.SetRe(re);
        wbwk.RenderKC(re);
    }
    catch(TException& ex)
    {
        fRespondErr(ex.error_info(), typeid(ex).name());
    }
    catch(std::exception& ex)
    {
        fRespondErr(ex.what(), typeid(ex).name());
    }
    catch(const char* ex)
    {
        fRespondErr(ex, "char");
    }
    catch(...)
    {
        fRespondErr("", "Unknown");
    }
}

// 静态页面处理
void KCWebSrvProxy::StaticPage(IWSProxyRequestCB& r)
{
    try
    {
        // 主模块接口
        IKCWebWork& wbwk = m_WebMainRef.getServiceSafe<IKCWebWork>();
        // 执行页面请求
        CWebRequestRespond re(*this, r, m_srvCB);
        r.SetRe(re);
        wbwk.StaticPage(re);
    }
    catch (...) {}
}

// 打包Web链接
IKCWebLongConn& KCWebSrvProxy::PackWebConn(IBaseRequestRespond& re, EWebConnType ewct)
{
    TKCWebConn *pConn = nullptr;
    // 按分类创建
    CWebRequestRespond& reWeb = dynamic_cast<CWebRequestRespond&>(re);
    if (ewctWebsocket == ewct) pConn = new TWebSrvConnWebsocket(*this, reWeb.m_recb);
    else if (ewctSSE == ewct) pConn = new TWebSrvConnSSE(*this, reWeb.m_recb);
    else throw std::runtime_error("EWebConnType error");
    boost::shared_ptr<TKCWebConn> ConnPtr(pConn);
    // 保存
    {
        boost::unique_lock<boost::shared_mutex> lck(m_mtxWC);
        auto it = m_wcs.find(pConn->GetID());
        if (m_wcs.end() != it) throw std::runtime_error("connect existsed");
        else m_wcs.insert(make_pair(pConn->GetID(), ConnPtr));
    }
    return *pConn;
}

// 释放web连接
void KCWebSrvProxy::ReleaseWebConn(IKCWebLongConn& wc)
{
    boost::unique_lock<boost::shared_mutex> lck(m_mtxWC);
    auto it = m_wcs.find(wc.GetID());
    if (m_wcs.end() != it) m_wcs.erase(it);
}

// 虚拟目录
unsigned KCWebSrvProxy::VirtualPathCount(void)
{
    return static_cast<unsigned>(m_vPathVct.size());
}
const char* KCWebSrvProxy::GetVirtualPath(unsigned pos)
{
    if (m_vPathVct.size() < pos)
    {
        auto it = m_vPath.find(m_vPathVct[pos]);
        if (m_vPath.end() != it) return it->second.c_str();
    }
    return nullptr;
}
const char* KCWebSrvProxy::GetVirtualPathUri(unsigned pos)
{
    if (m_vPathVct.size() < pos)
        return m_vPathVct[pos].c_str();
    return nullptr;
}

// 得到url对应的本地完整文件名
const char* KCWebSrvProxy::GetUrlLocalPath(const char* uri)
{
    /*
    // 通过全局配置获取
    const char* sLocal = m_WebApiWrkRef.getServiceSafe<IKCWebApiWork>().GetLocalFilename(uri);
    if (nullptr != sLocal && strlen(sLocal) > 0) return sLocal;

    // 通过网站虚拟目录获取
    string sLocalFile = string(m_srvCB.pgPath()) + CUtilFunc::PCharSafeToStr(uri);
    string sUri = CUtilFunc::PCharSafeToStr(uri);
    for (auto &pth : m_vPath)
    {
        string sVirPth = pth.first;
        string sLocalPth = pth.second;
        if (sUri == sVirPth)
            sLocalFile = sLocalPth;
        else if (algorithm::istarts_with(sUri, sVirPth + "/"))
            sLocalFile = algorithm::ireplace_first_copy(sUri, sVirPth, sLocalPth);
    }
    // 判断本地文件是否存在
    static thread_local string sResult;
    // sResult = CUtilFunc::ToAbsPath(sLocalFile, m_srvCB.pgPath());
    sResult = CUtilFunc::ToAbsPath(sLocalFile, m_srvCB.fxPath());
    if (!boost::filesystem::exists(sResult))
    {
        string sErr = (boost::format("%s %s (%s)") % GetContext().getHint("Don_t_exists_file_") % sResult % sLocalFile).str();
        m_WebMainRef.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__);
    }
    return sResult.c_str();
    */

    string sUri = CUtilFunc::PCharSafeToStr(uri);
    static thread_local string sResult;
    sResult.clear();

    // 从配置的虚拟路径列表里获取本地路径
    string sLocalFile = CUtilFunc::TransVPathToLocal(m_vPath, sUri);
    if (!sLocalFile.empty())
    {
        // sResult = CUtilFunc::ToAbsPath(sLocalFile, m_context.getWebsiteRootPath());
        sResult = CUtilFunc::PCharSafeToStr(m_BundleContextIF.transCfgPathToFullPath(sLocalFile.c_str()));
        // 判断本地文件是否存在
        if (boost::filesystem::exists(sResult)) return sResult.c_str();
        else
        {
            string sErr = (boost::format("%s %s\n%s (%s)") % m_BundleContextIF.getHint("Don_t_exists_file_") % sUri % sResult % sLocalFile).str();
            m_BundleContextIF.WriteLogWarning(sErr.c_str(), __CURR_CODE_PLACE_C__);
        }
    }
    // 在网站根目录下找
    sResult = m_srvCB.pgPath() + string("/") + sUri;
    if (boost::filesystem::exists(sResult)) return sResult.c_str();
    else
    {
        string sErr = (boost::format("%s %s\n%s") % m_BundleContextIF.getHint("Don_t_exists_file_") % sUri % sResult).str();
        m_BundleContextIF.WriteLogWarning(sErr.c_str(), __CURR_CODE_PLACE_C__);
    }
    // 通过全局配置获取
    return m_WebApiWrkRef.getServiceSafe<IKCWebApiWork>().GetUrlLocalPath(uri);
}

// 得到网页根路径
const char* KCWebSrvProxy::GetUrlPageRootPath(const char* uri)
{
    // 主网站虚拟路径
    static thread_local string sResult;
    sResult = CUtilFunc::GetUrlRootByVPath(m_vPath, CUtilFunc::PCharSafeToStr(uri));
    // auto iPos = sResult.find_first_of("/\\");
    // if (string::npos != iPos)
    // {
    //     iPos = sResult.find_first_of("/\\", iPos + 1);
    //     if (string::npos != iPos) sResult = sResult.substr(0, iPos);
    // }
    // else sResult = "/";
    // auto it = m_vPath.find(sResult);
    // if (m_vPath.end() == it) sResult = "/";
    if (sResult.size() > 1) return sResult.c_str();

    // 后端api配置目录
    const char* sKcApi = m_WebApiWrkRef.getServiceSafe<IKCWebApiWork>().GetUrlPageRootPath(uri);
    if (nullptr != sKcApi && strlen(sKcApi) > 1) return sKcApi;
    // 默认
    return "/";
}

// 得到网站或应用根路径
const char* KCWebSrvProxy::GetWebsiteRootPath(void)
{
    return SrvCB().websitePath();
}
// 得到主平台根路径
const char* KCWebSrvProxy::GetPlatformRootPath(void)
{
    return SrvCB().platformPath();
}
// 得到web服务程序或应用程序的根目录
const char* KCWebSrvProxy::GetApPath(void)
{
    return SrvCB().apPath();
}

// 链接关闭前
void KCWebSrvProxy::PreClose(IWSProxyConnectCB& /*c*/)
{
    // todo: 连接断开
}

// 得到模块上下文
IBundleContext& KCWebSrvProxy::GetContext(void) const
{
    return m_BundleContextIF;
}

// 获取链接的弱指针
boost::weak_ptr<IKCWebLongConn> KCWebSrvProxy::GetConnWPtr(long cid)
{
    boost::shared_lock<boost::shared_mutex> lck(m_mtxWC);
    auto it = m_wcs.find(cid);
    if (m_wcs.end() != it) return boost::weak_ptr<IKCWebLongConn>(it->second);
    return boost::weak_ptr<IKCWebLongConn>();
}

// web服务回调接口
IWSProxyServerCB& KCWebSrvProxy::SrvCB(void)
{
    return m_srvCB;
}
