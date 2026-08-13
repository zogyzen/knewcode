#include "work_proxy_cb.h"
#include "api_work.h"

////////////////////////////////////////////////////////////////////////////////
// 请求和应答回调类
CWSProxyRequestCB::CWSProxyRequestCB(CApiWork& own, IKCWebSrvProxy& proxy, intptr_t r)
    : m_own(own), m_proxy(proxy), m_hRe(r), m_hConn(own.GetRequestConn(r))
{
}
CWSProxyRequestCB::~CWSProxyRequestCB(void)
{
}

// 设置请求应答接口
void CWSProxyRequestCB::SetRe(ISrcRequestRespond& re)
{
    m_re = &re;
}

// 得到本地完整文件名
const char* CWSProxyRequestCB::GetLocalFilename(void) const
{
    return GetSingleInfo("filename");
}
const char* CWSProxyRequestCB::GetLocalFilename(const char* uri) const
{
    string sUri = CUtilFunc::PCharSafeToStr(uri);
    static thread_local string sResult;
    sResult.clear();

    // 从配置的虚拟路径列表里获取本地路径
    string sLocalFile = CUtilFunc::TransVPathToLocal(m_own.m_vPath, sUri);
    if (!sLocalFile.empty())
    {
        sResult = CUtilFunc::PCharSafeToStr(m_own.GetContext().transCfgPathToFullPath(sLocalFile.c_str()));
        // 判断本地文件是否存在
        if (!boost::filesystem::exists(sResult))
        {
            string sErr = (boost::format("%s %s\n%s (%s)") % m_own.GetContext().getHint("Don_t_exists_file_") % sUri % sResult % sLocalFile).str();
            m_own.GetContext().WriteLogWarning(sErr.c_str(), __CURR_CODE_PLACE_C__);
        }
        // 有值则返回
        return sResult.c_str();
    }
    // 在网站根目录下找
    else
    {
        sResult = m_own.m_WebsitePath + "/" + sUri;
        return sResult.c_str();
    }
}

// 得到网络根路径
const char* CWSProxyRequestCB::GetUrlPageRootPath(const char* /*uri*/) const
{
    // todo: 推迟到“kc_websrv_proxy”模块查找
    return nullptr;
}

// 虚拟目录
unsigned CWSProxyRequestCB::VirtualPathCount(void) const
{
    // todo: 推迟到“kc_websrv_proxy”模块
    return 0;
}
const char* CWSProxyRequestCB::GetVirtualPath(unsigned) const
{
    // todo: 推迟到“kc_websrv_proxy”模块
    return nullptr;
}
const char* CWSProxyRequestCB::GetVirtualPathUri(unsigned) const
{
    // todo: 推迟到“kc_websrv_proxy”模块
    return nullptr;
}

// 判断是否ssl
bool CWSProxyRequestCB::IsSSL(void) const
{
    return m_own.IsSSL(m_own.GetRequestConn(m_hRe));
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
            if ("Version" == sName)
                pResult = CUtilFunc::ToKeepStr<1>((boost::format("%s | %s") % m_own.GetSrvInfo("Version") % m_proxy.GetContext().VersionInfo()).str());
            else if ("MainExeModRoot" == sName) pResult = m_own.m_WebSrvPath.c_str();
            else if ("KnewcodeCfgFile" == sName) pResult = m_own.m_CfgFile.c_str();
            else if ("KnewcodeRoot" == sName) pResult = m_own.m_FxPath.c_str();
            else if ("DocumentRoot" == sName) pResult = m_own.m_PgPath.c_str();
            else if ("WebsiteRoot" == sName) pResult = m_own.m_WebsitePath.c_str();
            else if ("PlatformRoot" == sName)
                pResult = CUtilFunc::ToKeepStr<2>(boost::filesystem::path(m_own.m_FxPath).parent_path().string());
            else if ("VirtualPath" == sName) pResult = m_own.m_virtualPath.c_str();
            else pResult = m_own.GetSrvInfo(sName.c_str());
            return nullptr != pResult;
        };
        // 链接信息
        auto fGetConnInfo = [&]()
        {
            if ("connection_client_ip" == sName) pResult = m_own.GetConnInfo(m_hConn, "client_ip");
            else if ("connection_local_ip" == sName) pResult = m_own.GetConnInfo(m_hConn, "local_ip");
            else if ("connection_client_port" == sName) pResult = m_own.GetConnInfo(m_hConn, "client_port");
            else if ("connection_local_port" == sName) pResult = m_own.GetConnInfo(m_hConn, "local_port");
            else if ("UniqueConnID" == sName)
                pResult = CUtilFunc::ToKeepStr<3>(CUtilFunc::GetUniqueConnID(m_proxy.GetContext().GetSrvID(), atoi(m_own.GetConnInfo(m_hConn, "connection_id"))));
            else if ("ClientIpPort" == sName)
                pResult = CUtilFunc::ToKeepStr<4>((boost::format("%s:%s") % m_own.GetConnInfo(m_hConn, "client_ip") % m_own.GetConnInfo(m_hConn, "client_port")).str());
            else pResult = m_own.GetConnInfo(m_hConn, sName.c_str());
            return nullptr != pResult;
        };
        // 其他请求信息
        auto fGetRequestHeader = [&]()
        {
            string sNameHeader = algorithm::replace_all_copy(sName, "_", "-");
            pResult = GetRequestHeader(sNameHeader.c_str());
            return nullptr != pResult;
        };
                // 其他请求信息
        auto fGetRequestOther = [&]()
        {
            if ("content_type" == sName)
            {
                pResult = GetRequestHeader(c_WebHeader_ContentType);
                if (nullptr == pResult || strlen(pResult) == 0) pResult = m_own.GetRequestOther(m_hRe, "content_type");
            }
            else if ("content_length" == sName)
            {
                pResult = GetRequestHeader(c_WebHeader_ContentLength);
                if (nullptr == pResult || strlen(pResult) == 0) pResult = m_own.GetRequestOther(m_hRe, "content_length");
            }
            else if ("client_host_port" == sName) pResult = CUtilFunc::ToKeepStr<5>(GetSingleInfo("client_ip") + string(":") + GetSingleInfo("client_port"));
            else if ("server_host_port" == sName) pResult = CUtilFunc::ToKeepStr<6>(GetSingleInfo("local_ip") + string(":") + GetSingleInfo("local_port"));
            else if ("PostArg" == sName && nullptr != m_re) pResult = m_re->GetPostArgStr();
            else if ("KCAllConstInfo" == sName && nullptr != m_re) pResult = m_re->GetAllInfo();
            else pResult = m_own.GetRequestOther(m_hRe, sName.c_str());
            return nullptr != pResult;
        };

        fGetRequestOther() || fGetRequestHeader() || fGetConnInfo() || fGetSrvInfo();
    }
    catch (...) {}
    return CUtilFunc::PCharSafeToPChar(pResult, pDef);
}

// 是否post请求
bool CWSProxyRequestCB::IsPost(void) const
{
    return boost::algorithm::to_upper_copy(string(m_own.GetRequestOther(m_hRe, "method"))) == "POST";
}

// 按名称得到请求头
const char* CWSProxyRequestCB::GetRequestHeader(const char* name) const
{
    string sName = CUtilFunc::PCharSafeToStr(name);
    boost::algorithm::replace_all(sName, "_", "-");
    return m_own.GetRequestHead(m_hRe, sName.c_str());
}

// 获取客户端post输入
unsigned CWSProxyRequestCB::GetClientBlock(char* pBuf, unsigned iPostLen) const
{
    if (nullptr != pBuf && iPostLen > 0)
    {
        auto [len, buf] = m_own.GetRequestBody(m_hRe);
        if (len > 0) memcpy(pBuf, buf, std::min(len, iPostLen));
        return len;
    }
    else return 0;
}

// 设置应答类型
void CWSProxyRequestCB::SetContentType(const char* ct)
{
    if (nullptr != ct) m_own.SetRespondHead(m_hRe, c_WebHeader_ContentType, ct);
}

// 设置响应状态
void CWSProxyRequestCB::SetResponseStatus(int iStt)
{
    m_own.SetRespondOther(m_hRe, "respond_status", std::to_string(iStt));
}

// 按名称添加、删除响应头
void CWSProxyRequestCB::AddResponseHeader(const char* name, const char* val)
{
    if (nullptr != name /*&& nullptr != val*/)
    {
        string sName = name;
        boost::algorithm::replace_all(sName, "_", "-");
        m_own.SetRespondHead(m_hRe, sName.c_str(), CUtilFunc::PCharSafeToStr(val));
    }
}
void CWSProxyRequestCB::DelResponseHeader(const char* name)
{
    if (nullptr != name)
    {
        string sName = name;
        boost::algorithm::replace_all(sName, "_", "-");
        m_own.SetRespondHead(m_hRe, sName.c_str(), "");
    }
}

// 输出网页内容
bool CWSProxyRequestCB::AddResponseBody(const char* buf, int nbyte)
{
    if (nullptr != buf)
    {
        if (nbyte > 0)
            return m_own.SetRespondBody(m_hRe, string(buf, nbyte)) == 0;
        else
            return m_own.SetRespondBody(m_hRe, buf) == 0;
    }
    return false;
}
// 提交响应（将这段时间，服务器端的输出，同时推给客户端）
void CWSProxyRequestCB::CommitResponse(void)
{
    m_own.CommitResponse(m_hRe);
}
