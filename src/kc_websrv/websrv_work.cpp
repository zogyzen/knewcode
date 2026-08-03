#include "websrv_work.h"
#include "work_proxy_cb.h"

// 写日志
void WriteLog(string sFlag, string sErr, string sPos)
{
    cout << sFlag << " - " << sErr << endl;
    if (g_work.get() != nullptr && !g_work->m_proxy.GetContext().WriteLogFatal(sFlag.c_str(), sPos.c_str(), sErr.c_str()))
        CTempLog::WriteInDir(boost::filesystem::exists(g_work->m_FxPath) ? g_work->m_FxPath : "", sFlag, sPos, sErr);
}

////////////////////////////////////////////////////////////////////////////////
// 链接回调类
class CWSProxyConnectCB : public IWSProxyConnectCB
{
public:
    CWSProxyConnectCB(CWebSrvWork& own, KCSrv::KcSrvConnectPtr c, KCSrv::KcSrvRespondPtr r) : m_own(own), m_conr(c), m_res(r) {};

    // 获取连接ID
    long CALL_TYPE GetID(void) const override
    {
        return m_conr->GetID();
    }

    // 判断是否ssl
    bool CALL_TYPE IsSSL(void) const override
    {
        return m_conr->GetSrvHttp().IsSSL();
    }

    // 设置长连接
    void CALL_TYPE SetKeepalive(bool b) override
    {
        // m_own.SetKeepalive(m_conr, b);
    }
    // 设置断开的回调函数
    void CALL_TYPE SetBreakCB(void* cdb, FBreakCB fcb) override
    {
        // m_own.SetBreakConnCB(m_conr, reinterpret_cast<intptr_t>(cdb), reinterpret_cast<FBreakConnCBFromOwn>(fcb));
    }
    // 关闭连接
    void CALL_TYPE CloseConn(void) override
    {
        m_conr->CloseConn();
    }
    // 连接是否关闭
    bool isCloseConn(void) const override
    {
        return !m_conr->IsOpen();
    }
    // 请求应答上下文的回调
    IWSProxyRequestCB& CALL_TYPE MakeReCB(void) override
    {
        return *new CWSProxyRequestCB(m_own, m_own.m_proxy, m_res);
    }
    void CALL_TYPE ReleaseReCB(IWSProxyRequestCB& rcb) override
    {
        delete dynamic_cast<CWSProxyRequestCB*>(&rcb);
    }
    // 重置链接时间
    void CALL_TYPE ResetTime(unsigned ms) override
    {
        // m_own.ResetConnTime(m_conr, ms);
    }
    // 发送
    int CALL_TYPE Send(const char* buf, unsigned len) override
    {
        // return m_own.SendByConn(m_conr, buf, len);
        return 0;
    }
    // socket发送
    int CALL_TYPE SockSend(const char* buf, size_t& sz) override
    {
        // return m_own.SockSendByConn(m_conr, buf, static_cast<unsigned>(sz));
        return 0;
    }
    // websocket接收
    int CALL_TYPE WscRecv(char* buf, size_t& sz) override
    {
        // return m_own.WscRecvByConn(m_conr, buf, static_cast<unsigned>(sz));
        return 0;
    }

private:
    CWebSrvWork& m_own;
    KCSrv::KcSrvConnectPtr m_conr;
    KCSrv::KcSrvRespondPtr m_res;
};

////////////////////////////////////////////////////////////////////////////////
// web服务回调类
class CWSProxyServerCB : public IWSProxyServerCB
{
public:
    CWSProxyServerCB(CWebSrvWork& own) : m_own(own) {};

    // 宿主应用程序名称
    const char* CALL_TYPE OwnName(void) const override
    {
        return m_own.m_ownName.c_str();
    }

    // 宿主应用程序版本
    const char* CALL_TYPE OwnVersion(void) const override
    {
        return m_own.m_ownVersion.c_str();
    }

    // web服务程序的根目录
    const char* CALL_TYPE apPath(void) const override
    {
        return m_own.m_MainExecPath.c_str();
    }
    // 主平台根目录
    const char* CALL_TYPE platformPath(void) const override
    {
        return m_own.m_PlatformPath.c_str();
    }
    // 网站或应用根路径
    const char* CALL_TYPE websitePath(void) const override
    {
        return m_own.m_WebsitePath.c_str();
    }
    // KC系统根目录
    const char* CALL_TYPE fxPath(void) const override
    {
        return m_own.m_FxPath.c_str();
    }
    // 主页根目录
    const char* CALL_TYPE pgPath(void) const override
    {
        return m_own.m_PgPath.c_str();
    }
    // 配置文件完整路径
    const char* CALL_TYPE cfgFile(void) const override
    {
        return m_own.m_CfgFile.c_str();
    }

    // web虚拟目录
    const char* CALL_TYPE VirtualPath(void) const override
    {
        return m_own.m_virtualPath.c_str();
    }

    // 链接
    IWSProxyConnectCB& CALL_TYPE MakeConnCB(IWSProxyRequestCB& recb) override
    {
        auto r = dynamic_cast<CWSProxyRequestCB&>(recb).m_res;
        return *new CWSProxyConnectCB(m_own, r->m_request->m_connect, 0);
    }
    void CALL_TYPE ReleaseConnCB(IWSProxyConnectCB& cncb) override
    {
        delete dynamic_cast<CWSProxyRequestCB*>(&cncb);
    }

    // 判断是否断线状态
    bool isDisconn(int stt) const override
    {
        // return m_own.IsDisconnByErrCodeCB(stt);
        return false;
    }
    // 获取状态的错误信息
    virtual const char* GetStatus(int stt) const override
    {
        // return m_own.GetStatusByErrCodeCB(stt);
        return "";
    }

private:
    CWebSrvWork& m_own;
};

////////////////////////////////////////////////////////////////////////////////
// 内置web服务器 工作类
CWebSrvWork::CWebSrvWork(string sExe, string sCfg)
    : m_exePath(sExe), m_exeParm(sCfg), m_ownName("kc_websrv"), m_ownVersion(m_ownName + " " + CUtilFunc::KcVersionForShow())
    // 可执行文件根目录
    , m_MainExecPath([&](){
        string sPth = boost::filesystem::path(sExe).parent_path().string();
        if (sPth.empty()) sPth = "./";
        if (boost::filesystem::exists(sPth)) sPth = boost::filesystem::canonical(sPth).string();
        cout << "\t" << sExe << " => " << sPth << endl;
        return sPth;
    }())
    // knewcode模块所在目录
    , m_FxPath(m_MainExecPath)
    // 网站或应用配置文件
    , m_CfgFile([&](){
        string sPth = sCfg;
        // 以当前路径为基准，找配置文件
        if (boost::filesystem::exists(sPth)) sPth = boost::filesystem::canonical(sPth).string();
        // 以主程序路径为基准，找配置文件
        else sPth = CUtilFunc::ToAbsPath(sPth, m_MainExecPath);
        if (!boost::filesystem::exists(sPth)) throw std::runtime_error("Configuration file error: " + sPth);
        cout << "\t" << sCfg << " => " << sPth << endl;
        return sPth;
    }())
    // 网站或应用的根目录，默认为“网站或应用配置文件”所在目录
    , m_WebsitePath(boost::filesystem::path(m_CfgFile).parent_path().string())
    // 主平台目录，默认为“knewcode模块所在目录”的上一层目录
    , m_PlatformPath(boost::filesystem::path(m_FxPath).parent_path().string())
    // 主页目录
    , m_PgPath(m_WebsitePath + "/frontend/dist")
    // 加载代理
    , m_srvCB(*new CWSProxyServerCB(*this)), m_load(m_srvCB), m_proxy(m_load.Proxy())
{
}
// 主框架
std::shared_ptr<CWebSrvWork> g_work;

// 初始化
void CWebSrvWork::Init(void)
{
    // 启动代理
    string sMsg = (boost::format("\t MainExeRoot: \t%s (%s) \n\t PlatformRoot: \t%s \n\t WebsiteRoot: \t%s \n\t KnewcodeRoot: \t%s \n\t DocumentRoot: \t%s \n\t KnewcodeCfgFile: \t%s (%s)\n")
                   % m_MainExecPath % m_exePath % m_PlatformPath % m_WebsitePath % m_FxPath % m_PgPath % m_CfgFile % m_exeParm
                   ).str();
    cout << "*[knewcode] load knewcode mod begin \n" << sMsg << endl;
    m_proxy.Init();
    // 获取内置web服务器配置
    auto &cntx = dynamic_cast<IBundleContextEx&>(m_proxy.GetContext());
    auto &cfgPt = cntx.CfgPt();
    cntx.WriteLogDebug(sMsg.c_str(), __CURR_CODE_PLACE_C__);
    // 端口配置
    unsigned short  portHttp = atoi(cntx.GetCfgInfo("Config.WebServer.port", "http", "0")),
                    portHttps = atoi(cntx.GetCfgInfo("Config.WebServer.port", "https", "0"));
    // 禁止访问文件的扩展名
    string sDeniedUrlExtName = cntx.GetCfgInfo("Config.WebServer.other", "deniedUrlExtName");
    std::vector<string> vctDeniedUrlExtName;
    boost::algorithm::split(vctDeniedUrlExtName, sDeniedUrlExtName, boost::is_any_of("|"));
    for (auto &str : vctDeniedUrlExtName) m_stDeniedUrlExtName.insert(boost::algorithm::trim_copy(str));
    // 主页配置
    string sDomains;
    auto fGetPageCfg = [&](const property_tree::ptree &ptCfg, TMainPageCfg& pgCfg, string domainName = "127.0.0.1")
    {
        for(const property_tree::ptree::value_type &vt: ptCfg.get_child(c_RESTful_xmlattr))
        try
        {
            string sName = vt.first;
            string sVal = vt.second.get_value<string>();
            // cout << sName << " = " << sVal << endl;
            // 系统名称
            if ("sysName" == sName) pgCfg.m_sysName = sVal;
            // 本地根路径
            else if ("root" == sName) pgCfg.m_rootPath = cntx.transCfgPathToFullPath(boost::algorithm::trim_right_copy_if(sVal, boost::is_any_of("/")).c_str());
            // 主页
            else if ("index" == sName) pgCfg.m_indexPage = sVal;
            // 错误页
            else if ('_' == sName[0] && sName.size() > 1 && ('4' == sName[1] || '5' == sName[1]))
            {
                int iErrCode = atoi(sName.substr(1).c_str());
                pgCfg.m_errPage.insert(std::make_pair(iErrCode, sVal));
            }
        }
        catch (...) {}
        sDomains += (boost::format("\t http://%s:%d/ \t | \t https://%s:%d/ \n") % domainName % portHttp % domainName % portHttps).str();
    };
    if (cfgPt.get_child_optional("Config.WebServer.page"))
        fGetPageCfg(cfgPt.get_child("Config.WebServer.page"), m_mainHost.m_pageCfg);
    const_cast<string&>(m_PgPath) = m_mainHost.m_pageCfg.m_rootPath;
    // 虚拟目录
    auto fGetVirtualPath = [&](const property_tree::ptree &ptCfg, map<string, string> &vPathMap, vector<string> &vPathVct)
    {
        // 循环每个子项
        for(const property_tree::ptree::value_type &vt: ptCfg)
        try
        {
            string sName = vt.first;
            if (c_RESTful_xmlcomment != sName && c_RESTful_xmlattr != sName)
            {
                string sUrl = vt.second.get<string>(string(c_RESTful_xmlattr) + "." + c_RESTful_uri);
                string sLocal = cntx.transCfgPathToFullPath(boost::algorithm::trim_right_copy_if(vt.second.get<string>(string(c_RESTful_xmlattr) + ".local"), boost::is_any_of("/")).c_str());
                vPathMap.insert(make_pair(sUrl, sLocal));
                vPathVct.push_back(sUrl);
            }
        }
        catch (...) {}
    };
    if (cfgPt.get_child_optional("Config.WebServer.virtualPath"))
        fGetVirtualPath(cfgPt.get_child("Config.WebServer.virtualPath"), m_mainHost.m_pageCfg.m_vPath, m_mainHost.m_pageCfg.m_vPathVct);
    for (auto &vPth : m_mainHost.m_pageCfg.m_vPath) const_cast<string&>(m_virtualPath) += vPth.first + "\t" + vPth.second;
    // 虚拟主机
    auto fGetVirtualHost = [&](string sNode, map<string, TVirtualHost>& vHostMap)
    {
        // 循环每个子项
        if (cfgPt.get_child_optional(sNode))
            for(const property_tree::ptree::value_type &vt: cfgPt.get_child(sNode))
            try
            {
                TVirtualHost vHost;
                vHost.m_name = vt.first;
                if (c_RESTful_xmlcomment != vHost.m_name && c_RESTful_xmlattr != vHost.m_name)
                {
                    vHost.m_domainName = vt.second.get<string>(string(c_RESTful_xmlattr) + ".domainName");
                    fGetPageCfg(vt.second.get_child("page"), vHost.m_pageCfg, vHost.m_domainName);
                    fGetVirtualPath(vt.second.get_child("virtualPath"), vHost.m_pageCfg.m_vPath, m_mainHost.m_pageCfg.m_vPathVct);
                    vHostMap.insert(make_pair(vHost.m_domainName, vHost));
                }
            }
            catch (...) {}
    };
    fGetVirtualHost("Config.WebServer.virtualHost", m_virtualHost);
    // 创建内置web服务器
    auto self(this->shared_from_this());
    m_kcSrv.reset(new KCSrv::KcSrvMainExec(*this, /*cntx.VersionInfo()*/CUtilFunc::KcVersionForFullInfo(),
                                           [this, self](KCSrv::KcSrvRespondPtr res){ this->Work(res); }));
    // 内置web服务器的参数
    m_kcSrv->m_parm.portHttp = portHttp;
    m_kcSrv->m_parm.portHttps = portHttps;
    m_kcSrv->m_parm.threadCount = atoi(cntx.GetCfgInfo("Config.WebServer", "threadCount", "64"));
    m_kcSrv->m_parm.sslKey = cntx.transCfgPathToFullPath(cntx.GetCfgInfo("Config.WebServer.ssl", "key", "./ssl/private.key"));
    m_kcSrv->m_parm.sslCert = cntx.transCfgPathToFullPath(cntx.GetCfgInfo("Config.WebServer.ssl", "cert", "./ssl/fullchain.pem"));
    // 启动内置web服务器
    m_kcSrv->Start();
    // 日志
    sMsg = (boost::format("\t PgPath: \t%s \n\t sslKey: \t%s \n\t sslCert: \t%s \n\t threadCount=%d \t portHttp=%d \t portHttps=%d \n%s")
                % m_PgPath % m_kcSrv->m_parm.sslKey % m_kcSrv->m_parm.sslCert
                % m_kcSrv->m_parm.threadCount % m_kcSrv->m_parm.portHttp % m_kcSrv->m_parm.portHttps
                % sDomains
           ).str();
    cout << "*[knewcode] load knewcode mod success \n" << sMsg << endl;
    cntx.WriteLogDebug(sMsg.c_str(), __CURR_CODE_PLACE_C__);
}

// 释放
void CWebSrvWork::Free(void)
{
    if (m_kcSrv.get() != nullptr) m_kcSrv->Stop();
    m_proxy.Free();
    m_kcSrv.reset();
}

// 处理请求
void CWebSrvWork::Work(KCSrv::KcSrvRespondPtr res)
{
    // res->m_body = "Hello Knewcode";
    // return 0;

    CWSProxyRequestCB reqCB(*this, m_proxy, res);
    // 匹配主机
    string sHostName = res->m_request->m_hostName;
    const TVirtualHost *pHost = &m_mainHost;
    auto it = m_virtualHost.find(sHostName);
    if (m_virtualHost.end() != it) pHost = &it->second;
    res->m_request->m_attachParm = pHost;
    // 返回静态页面
    if (".kc" != res->m_request->m_extName)
    {
        // 正常请求
        if (!res->m_request->m_ContentType.empty())
        {
            // 静态页面文件
            string sPageFile = pHost->m_pageCfg.GetLocalPath(res->m_request->m_uri);
            if (!sPageFile.empty())
            {
                // 错误页
                auto fErrDeal = [&](const int iErrCode)
                {
                    string sErrPage;
                    auto fGetErrPage = [&](int iCode)
                    {
                        auto it = pHost->m_pageCfg.m_errPage.find(iCode);
                        if (pHost->m_pageCfg.m_errPage.end() != it) sErrPage = it->second;
                        return !sErrPage.empty();
                    };
                    fGetErrPage(iErrCode) || fGetErrPage(iErrCode / 10) || fGetErrPage(iErrCode / 100);
                    sErrPage = pHost->m_pageCfg.m_rootPath + "/" + (sErrPage.empty() ? std::to_string(iErrCode) + ".html" : sErrPage);
                    return make_tuple(sErrPage, pHost->m_pageCfg.m_sysName);
                };
                // 禁止的扩展名
                if (m_stDeniedUrlExtName.find(res->m_request->m_extName) != m_stDeniedUrlExtName.end())
                    // 显示403页
                    res->SetErrorPage(sPageFile, 403, fErrDeal);
                // 静态页面
                else
                    // 显示静态页面
                    res->SetStaticPage(sPageFile, fErrDeal);
            }
            m_proxy.StaticPage(reqCB);
        }
    }
    // 调用后端api
    else m_proxy.Work(reqCB);
}
