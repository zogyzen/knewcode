#include "request_respond.h"
#include "kc_websrv_proxy.h"

////////////////////////////////////////////////////////////////////////////////
// CWebRequestRespond类
CWebRequestRespond::CWebRequestRespond(KCWebSrvProxy& own, IWSProxyRequestCB& r, IWSProxyServerCB& srv)
    : m_own(own), m_recb(r), m_srv(srv)
{
    m_recb.SetRe(*this);
}
CWebRequestRespond::~CWebRequestRespond()
{
    m_postBuf.reset();
}

// 判断是否ssl
bool CWebRequestRespond::IsSSL(void)
{
    return m_recb.IsSSL();
}

// 得到服务器端全部信息
const char* CWebRequestRespond::GetAllInfo(const char* endTag) const
{
    static thread_local string sAllInfo;
    sAllInfo.clear();
    for (string sName : c_arrWebRequestNames)
        if ("KCAllConstInfo" != sName)
            sAllInfo += sName + ": " + CUtilFunc::PCharSafeToStr(GetSingleInfo(sName.c_str())) + endTag;
    return sAllInfo.c_str();
}

// 得到服务器端各单个请求信息
const char* CWebRequestRespond::GetSingleInfo(const char* pName, const char* pDef) const
{
    return m_recb.GetSingleInfo(pName, pDef);
}

////////////////////////////请求部分///////////////////////////
// 得到本地完整文件名
const char* CWebRequestRespond::GetLocalFilename(void)
{
    // const char* sLocal = m_own.m_WebApiWrkRef.getServiceSafe<IKCWebApiWork>().GetLocalFilename(this->GetUriFilename());
    const char* pLocalFile = m_recb.GetLocalFilename();
    if (nullptr != pLocalFile && strlen(pLocalFile) > 0 && boost::filesystem::exists(string(pLocalFile)))
        return pLocalFile;
    return this->GetLocalFilename(this->GetUriFilename());
}
const char* CWebRequestRespond::GetLocalFilename(const char* uri)
{
    // return m_own.GetLocalFilename(uri);
    // 在web服务器配置的虚拟目录找；在web服务器的网站根目录下找
    const char* pLocalFile = m_recb.GetLocalFilename(uri);
    if (nullptr != pLocalFile && strlen(pLocalFile) > 0 && boost::filesystem::exists(string(pLocalFile)))
        return pLocalFile;
    // 在后端api配置的目录下找
    const char* pLocalMainApiFile =  m_own.m_WebApiWrkRef.getServiceSafe<IKCWebApiWork>().GetUrlLocalPath(uri);
    if (nullptr != pLocalMainApiFile && strlen(pLocalMainApiFile) > 0 && boost::filesystem::exists(string(pLocalMainApiFile)))
        return pLocalMainApiFile;
    return pLocalFile;
}

// 得到网络文件名
const char* CWebRequestRespond::GetUriFilename(void)
{
    return this->GetSingleInfo("uri");
}
// 得到网络路径
const char* CWebRequestRespond::GetUrlPagePath(void)
{
    return CUtilFunc::GetUrlPagePath(GetUriFilename());
}
// 得到网络根路径
const char* CWebRequestRespond::GetUrlPageRootPath(void)
{
    return GetUrlPageRootPath(GetUrlPagePath());
}
const char* CWebRequestRespond::GetUrlPageRootPath(const char* uri)
{
    // 在web服务器配置的虚拟目录货web服务器的网站根目录下找
    const char* pWebPth = m_recb.GetUrlPageRootPath(uri);
    if (nullptr != pWebPth && strlen(pWebPth) > 1) return pWebPth;
    // 在主网站或后端api目录下找
    return m_own.GetUrlPageRootPath(uri);
}

// 端口
int CWebRequestRespond::GetPort(void)
{
    int iPort = this->IsSSL() ? 443 : 80;
    try
    {
        iPort = lexical_cast<int>(GetSingleInfo("parsed_uri_port"));
    }
    catch(...) {}
    return iPort;
}

// GET的参数
const char* CWebRequestRespond::GetGetArgStr(void) const
{
    return GetSingleInfo("args");
}

// Post的参数
const char* CWebRequestRespond::GetPostArgType(void)
{
    if(m_recb.IsPost())
        return GetSingleInfo(c_WebHeader_ContentType);
    return "";
}
int CWebRequestRespond::GetPostArgLength(void)
{
    if(m_recb.IsPost())
        return atoi(GetSingleInfo(c_WebHeader_ContentLength));
    return 0;
}
int CWebRequestRespond::GetPostArgBuffer(char*& buf, int len)
{
    int iPostLen = GetPostArgLength();
    if (m_postBuf.get() == nullptr)
    {
        char *pBuf = new char[iPostLen + 1] { 0 };
        std::shared_ptr<char> BufPtr(pBuf);
        m_recb.GetClientBlock(pBuf, iPostLen);
        m_postBuf = BufPtr;
    }
    buf = m_postBuf.get();
    return len < 0 ? iPostLen : min(len, iPostLen);
}
const char* CWebRequestRespond::GetPostArgStr(void)
{
    char *postBuf = nullptr;
    GetPostArgBuffer(postBuf, GetPostArgLength());
    static thread_local string sResult;
    sResult = postBuf;
    return sResult.c_str();
}

// 得到网站本地完整根目录
const char* CWebRequestRespond::GetLocalRootPath(void)
{
    return m_srv.pgPath();
}

// 按名称得到请求头
const char* CWebRequestRespond::GetRequestHeader(const char* name) const
{
    return m_recb.GetRequestHeader(name);
}

// 虚拟目录
unsigned CWebRequestRespond::VirtualPathCount(void)
{
    auto iResult = m_recb.VirtualPathCount();
    return iResult > 0 ? iResult : m_own.VirtualPathCount();
}
const char* CWebRequestRespond::GetVirtualPath(unsigned pos)
{
    return m_recb.VirtualPathCount() > 0 ? m_recb.GetVirtualPath(pos) : m_own.GetVirtualPath(pos);
}
const char* CWebRequestRespond::GetVirtualPathUri(unsigned pos)
{
    return m_recb.VirtualPathCount() > 0 ? m_recb.GetVirtualPathUri(pos) : m_own.GetVirtualPathUri(pos);
}

// 全局常量
IActionData::EGlobalValTpe CWebRequestRespond::GetGlobalConstType(const char* pName) const
{
    IActionData::EGlobalValTpe eResult = IActionData::egtIsnot;
    string sName = CUtilFunc::PCharSafeToStr(pName);
    // 系统常量
    if (sName.substr(0, sizeof(c_RESTful_KCGlobalConst) - 1) == c_RESTful_KCGlobalConst)
        eResult = IActionData::egtGloConst;
    else if (sName.substr(0, sizeof(c_RESTful_KCConst) - 1) == c_RESTful_KCConst)
        eResult = IActionData::egtConst;
    return eResult;
}
const char* CWebRequestRespond::GetGlobalConst(const char* pName, const char* pDef) const
{
    static thread_local string sResult;
    sResult.clear();
    string sName = CUtilFunc::PCharSafeToStr(pName);
    IActionData::EGlobalValTpe eGloVTp = GetGlobalConstType(pName);
    if (IActionData::egtGloConst == eGloVTp || IActionData::egtConst == eGloVTp)
    {
        string sConst = sName.substr((IActionData::egtConst == eGloVTp ? sizeof(c_RESTful_KCConst) : sizeof(c_RESTful_KCGlobalConst)) - 1);
        // 服务器编号
        if (c_RESTful_srvID == sConst)
        {
            sResult = std::to_string(m_own.GetContext().GetSrvID());
            return sResult.c_str();
        }
        // 系统标志
        else if (c_RESTful_sysFlag == sConst)
            return m_own.GetContext().GetSysFlag("customer");
        // 进程编号
        else if (c_RESTful_KCPID == sConst)
        {
            sResult = std::to_string(this_process::get_id());
            return sResult.c_str();
        }
        // 链接编号
        else if (c_RESTful_KCConnectID == sConst)
        {
            sResult = (format("%d_%s") % this_process::get_id() % this->GetSingleInfo("connection_id")).str();
            return sResult.c_str();
        }
        // 服务器当前时间戳（毫秒）
        else if (c_RESTful_SrvTimestamp == sConst)
        {
            sResult = std::to_string(CUtilFunc::GetCurrentStampMS());
            return sResult.c_str();
        }
        // 服务器当前时间
        else if (c_RESTful_SrvDatatime == sConst)
        {
            sResult = CUtilFunc::TimeToStr();
            return sResult.c_str();
        }
        // Web常量
        else
            return this->GetSingleInfo(sConst.c_str(), pDef);
    }
    return nullptr;
}

////////////////////////////应答部分///////////////////////////
// 设置响应文本类型
bool CWebRequestRespond::SetResponseContentType(const char* ct)
{
    m_respond_type = CUtilFunc::PCharSafeToPChar(ct);
    //AddResponseHeader(c_WebHeader_ContentType, ct);
    m_recb.SetContentType(ct);
    return true;
}
const char* CWebRequestRespond::GetResponseContentType(void)
{
    return !m_respond_type.empty() ? m_respond_type.c_str() : CUtilFunc::PCharSafeToPChar(GetSingleInfo("content_type"));
}

// 响应状态
bool CWebRequestRespond::SetResponseStatus(int iStt)
{
    m_recb.SetResponseStatus(iStt);
    return true;
}

// 按名称添加、删除响应头
void CWebRequestRespond::AddResponseHeader(const char* name, const char* val)
{
    m_recb.AddResponseHeader(name, val);
}
void CWebRequestRespond::DelResponseHeader(const char* name)
{
    m_recb.DelResponseHeader(name);
}

// 输出网页内容
bool CWebRequestRespond::AddResponseBody(const char* buf, int nbyte)
{
    return m_recb.AddResponseBody(buf, nbyte);
}
// 提交响应（将这段时间，服务器端的输出，同时推给客户端）
void CWebRequestRespond::CommitResponse(void)
{
    // 提交到客户端
    m_recb.CommitResponse();
}
