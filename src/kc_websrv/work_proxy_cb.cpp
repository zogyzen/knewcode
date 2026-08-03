#include "work_proxy_cb.h"
#include "websrv_work.h"

////////////////////////////////////////////////////////////////////////////////
// 请求和应答回调类
CWSProxyRequestCB::CWSProxyRequestCB(CWebSrvWork& own, IKCWebSrvProxy& proxy, KCSrv::KcSrvRespondPtr r)
    : m_own(own), m_proxy(proxy), m_res(r)
{
}
CWSProxyRequestCB::~CWSProxyRequestCB(void)
{
}

// 设置请求应答接口
void CWSProxyRequestCB::SetRe(ISrcRequestRespond& re)
{
    m_srcRR = &re;
}

// 得到本地完整文件名
const char* CWSProxyRequestCB::GetLocalFilename(void) const
{
    return GetLocalFilename(m_res->m_request->m_uri.c_str());
}
const char* CWSProxyRequestCB::GetLocalFilename(const char* uri) const
{
    static thread_local string sResult;
    sResult.clear();
    auto &req = *m_res->m_request.get();
    auto pHost = any_cast<const CWebSrvWork::TVirtualHost*>(&req.m_attachParm);
    if (nullptr != pHost)
        sResult = (*pHost)->m_pageCfg.GetLocalPath(CUtilFunc::PCharSafeToStr(uri));
    return sResult.c_str();
}

// 得到网络根路径
const char* CWSProxyRequestCB::GetUrlPageRootPath(const char* uri) const
{
    // 主网站虚拟路径
    static thread_local string sResult;
    sResult = CUtilFunc::PCharSafeToStr(uri);
    auto &req = *m_res->m_request.get();
    auto pHost = any_cast<const CWebSrvWork::TVirtualHost*>(&req.m_attachParm);
    if (nullptr != pHost)
    {
        auto &vPath = (*pHost)->m_pageCfg.m_vPath;
        for (auto it = vPath.find(sResult); vPath.end() == it && sResult.size() > 1; it = vPath.find(sResult))
            sResult = boost::filesystem::path(sResult).parent_path().string();
        if (sResult.size() > 1) return sResult.c_str();
    }
    return "/";
}

// 虚拟目录
unsigned CWSProxyRequestCB::VirtualPathCount(void) const
{
    auto &req = *m_res->m_request.get();
    auto pHost = any_cast<const CWebSrvWork::TVirtualHost*>(&req.m_attachParm);
    if (nullptr != pHost)
        return (*pHost)->m_pageCfg.m_vPath.size();
    return 0;
}
const char* CWSProxyRequestCB::GetVirtualPath(unsigned pos) const
{
    auto &req = *m_res->m_request.get();
    auto pHost = any_cast<const CWebSrvWork::TVirtualHost*>(&req.m_attachParm);
    if (nullptr != pHost)
    {
        const auto &pgCfg = (*pHost)->m_pageCfg;
        if (pgCfg.m_vPathVct.size() < pos)
        {
            auto it = pgCfg.m_vPath.find(pgCfg.m_vPathVct[pos]);
            if (pgCfg.m_vPath.end() != it) return it->second.c_str();
        }
    }
    return nullptr;
}
const char* CWSProxyRequestCB::GetVirtualPathUri(unsigned pos) const
{
    auto &req = *m_res->m_request.get();
    auto pHost = any_cast<const CWebSrvWork::TVirtualHost*>(&req.m_attachParm);
    if (nullptr != pHost)
    {
        const auto &pgCfg = (*pHost)->m_pageCfg;
        if (pgCfg.m_vPathVct.size() < pos)
            return pgCfg.m_vPathVct[pos].c_str();
    }
    return nullptr;
}

// 判断是否ssl
bool CWSProxyRequestCB::IsSSL(void) const
{
    return m_res->m_request->m_connect->GetSrvHttp().IsSSL();
}

// 得到服务器端各单个请求信息
const char* CWSProxyRequestCB::GetSingleInfo(const char* pName, const char* pDef) const
{
    const char* pResult = nullptr;
    try
    {
        string sName = CUtilFunc::PCharSafeToStr(pName);
        // 服务器信息
        auto fGetSrvInfo = [&]()
        {
            if ("Name" == sName) pResult = c_KnewcodeName;
            else if ("Version" == sName)            // 版本
                pResult = CUtilFunc::ToKeepStr<1>(m_proxy.GetContext().VersionInfo());
            else if ("MainExeModRoot" == sName)     // 主目录
                pResult = m_own.m_MainExecPath.c_str();
            else if ("KnewcodeRoot" == sName)       // knewcode模块所在目录
                pResult = m_own.m_FxPath.c_str();
            else if ("PlatformRoot" == sName)       // 主平台目录，默认为“knewcode模块所在目录”的上一层目录
                pResult = m_own.m_PlatformPath.c_str();
            else if ("KnewcodeCfgFile" == sName)    // 网站或应用配置文件
                pResult = m_own.m_CfgFile.c_str();
            else if ("WebsiteRoot" == sName)        // 网站或应用的根目录，默认为“网站或应用配置文件”所在目录
                pResult = m_own.m_WebsitePath.c_str();
            else if ("DocumentRoot" == sName)       // 主页目录
                pResult = m_own.m_PgPath.c_str();
            else if ("VirtualPath" == sName)        // 虚拟目录
                pResult = m_own.m_virtualPath.c_str();
            // else if ("server_defn_name" == sName) pResult = m_srv.defn_name;
            // else if ("server_path" == sName) pResult = m_srv.path;
            // else if ("server_admin" == sName) pResult = m_srv.server_admin;
            // else if ("server_hostname" == sName) pResult = m_srv.server_hostname;
            // else if ("server_scheme" == sName) pResult = m_srv.server_scheme;
            // else if ("server_timeout" == sName) pResult = CUtilFunc::ToKeepStr<2>(m_srv.timeout);
            // else if ("server_keep_alive_timeout" == sName) pResult = CUtilFunc::ToKeepStr<3>(m_srv.keep_alive_timeout);
            // else if ("server_keep_alive" == sName) pResult = CUtilFunc::ToKeepStr<4>(m_srv.keep_alive);
            return nullptr != pResult;
        };
        // 链接信息
        auto fGetConnInfo = [&]()
        {
            auto &conn = *m_res->m_request->m_connect.get();
            if ("ProtocolType" == sName) pResult = conn.GetSrvHttp().IsSSL() ? "https" : "http";
            else if ("client_ip" == sName || "connection_client_ip" == sName)
                pResult = CUtilFunc::ToKeepStr<51>(conn.ClientIP());
            else if ("local_ip" == sName || "connection_local_ip" == sName)
                pResult = CUtilFunc::ToKeepStr<52>(conn.LocalIP());
            else if ("client_port" == sName || "connection_client_port" == sName)
                pResult = CUtilFunc::ToKeepStr<53>(conn.ClientPort());
            else if ("local_port" == sName || "connection_local_port" == sName)
                pResult = CUtilFunc::ToKeepStr<54>(conn.LocalPort());
            else if ("connection_id" == sName) pResult = CUtilFunc::ToKeepStr<55>(conn.GetID());
            // else if ("connection_keepalive" == sName) pResult = CUtilFunc::ToKeepStr<56>(conn.ngx_keepalive);
            // else if ("connection_keepalives" == sName) pResult = CUtilFunc::ToKeepStr<57>(conn.ngx_keepalive);
            else if ("UniqueConnID" == sName)
                pResult = CUtilFunc::ToKeepStr<58>(CUtilFunc::GetUniqueConnID(m_proxy.GetContext().GetSrvID(), conn.GetID()));
            else if ("ClientIpPort" == sName)
                pResult = CUtilFunc::ToKeepStr<59>((boost::format("%s:%s") % conn.ClientIP() % conn.ClientPort()));
            else if ("LocalIpPort" == sName)
                pResult = CUtilFunc::ToKeepStr<59>((boost::format("%s:%s") % conn.LocalIP() % conn.LocalPort()));
            // else if ("connection_handle" == sName) pResult = CUtilFunc::ToKeepStr<60>(hConn);
            // else if ("connection_remote_host" == sName) pResult = conn.remote_host;
            // else if ("connection_remote_logname" == sName) pResult = conn.remote_logname;
            // else if ("connection_local_host" == sName) pResult = conn.local_host;
            // else if ("connection_local_addr" == sName) pResult = CUtilFunc::ToKeepStr<61>((format("%X") % conn.local_addr).str());
            // else if ("connection_socket" == sName) pResult = CUtilFunc::ToKeepStr<62>((format("%X") % ap_get_conn_socket(&conn)).str());
            // else if ("connection_local_ipaddr_ptr" == sName && nullptr != conn.local_addr)
            //     pResult = CUtilFunc::ToKeepStr<63>((format("%X") % conn.local_addr->ipaddr_ptr).str());
            // else if ("connection_client_addr" == sName) pResult = CUtilFunc::ToKeepStr<64>((format("%X") % conn.client_addr).str());
            // else if ("connection_client_ipaddr_ptr" == sName && nullptr != conn.client_addr)
            //     pResult = CUtilFunc::ToKeepStr<65>((format("%X") % conn.client_addr->ipaddr_ptr).str());
            return nullptr != pResult;
        };
        // 其他请求信息
        auto fGetRequestHeader = [&]()
        {
            string sNameHeader = algorithm::replace_all_copy(sName, "_", "-");
            pResult = this->GetRequestHeader(sNameHeader.c_str());
            return nullptr != pResult;
        };
        // 其他请求信息
        auto fGetRequestOther = [&]()
        {
            auto &req = *m_res->m_request.get();
            if ("uri" == sName) pResult = req.m_uri.c_str();
            else if ("filename" == sName) pResult = GetLocalFilename();
            else if ("unparsed_uri" == sName) pResult = req.m_unparsed_uri.c_str();
            else if ("args" == sName) pResult = req.m_args.c_str();
            else if ("method" == sName) pResult = req.m_method.c_str();
            else if ("the_request" == sName) pResult = req.m_the_request.c_str();
            else if ("content_type" == sName) pResult = req.m_ContentType.c_str();
            else if ("content_length" == sName) pResult = CUtilFunc::ToKeepStr<102>(req.m_ContentLength);
            else if ("client_host_port" == sName) pResult = GetSingleInfo("ClientIpPort");
            else if ("server_host_port" == sName) pResult = GetSingleInfo("LocalIpPort");
            else if ("PostArg" == sName && nullptr != m_srcRR) pResult = m_srcRR->GetPostArgStr();
            else if ("KCAllConstInfo" == sName && nullptr != m_srcRR) pResult = m_srcRR->GetAllInfo();
            return nullptr != pResult;
        };
        // 执行
        fGetRequestOther() || fGetRequestHeader() || fGetConnInfo() || fGetSrvInfo();
    }
    catch (...) {}
    return CUtilFunc::PCharSafeToPChar(pResult, pDef);
}

// 是否post请求
bool CWSProxyRequestCB::IsPost(void) const
{
    return m_res->m_request->IsPost();
}

// 按名称得到请求头
const char* CWSProxyRequestCB::GetRequestHeader(const char* name) const
{
    return m_res->m_request->GetHead(CUtilFunc::PCharSafeToStr(name));
}

// 获取客户端post输入
unsigned CWSProxyRequestCB::GetClientBlock(char* pBuf, unsigned iPostLen) const
{
    if (nullptr != pBuf && iPostLen > 0)
    {
        unsigned len = static_cast<unsigned>(m_res->m_request->m_body.size());
        if (len > 0) memcpy(pBuf, m_res->m_request->m_body.data(), std::min(len, iPostLen));
        return len;
    }
    else
        return 0;
}

// 设置应答类型
void CWSProxyRequestCB::SetContentType(const char* ct)
{
    if (nullptr != ct && strlen(ct) > 0)
        m_res->SetHead(c_WebHeader_ContentType, ct);
}

// 设置响应状态
void CWSProxyRequestCB::SetResponseStatus(int iStt)
{
    m_res->m_status = iStt;
}

// 按名称添加、删除响应头
void CWSProxyRequestCB::AddResponseHeader(const char* name, const char* val)
{
    if (nullptr != name && strlen(name) > 0 && nullptr != val && strlen(val) > 0)
    {
        string sName = name, sVal = val;
        boost::algorithm::replace_all(sName, "_", "-");
        if (KCSrv::c_WebHeader_SetCookie == sName)
        {
            int iPos = sVal.find("=");
            if (string::npos == iPos)
                m_res->SetHead(sName, sVal);
            else
                m_res->AddCookie(sVal.substr(0, iPos), sVal.substr(iPos + 1));
        }
        else
            m_res->SetHead(sName, sVal);
    }
}
void CWSProxyRequestCB::DelResponseHeader(const char* name)
{
    if (nullptr != name && strlen(name) > 0)
    {
        string sName = name;
        boost::algorithm::replace_all(sName, "_", "-");
        m_res->DelHead(sName);
    }
}

// 输出网页内容
bool CWSProxyRequestCB::AddResponseBody(const char* buf, int nbyte)
{
    if (nullptr != buf)
    {
        if (nbyte > 0)
            m_res->m_body = string(buf, nbyte);
        else
            m_res->m_body = buf;
        return true;
    }
    return false;
}
// 提交响应
void CWSProxyRequestCB::CommitResponse(void)
{
}
