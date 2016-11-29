#include "std.h"
#include "request_respond.h"
#include "kc_web_work.h"

////////////////////////////////////////////////////////////////////////////////
// CKCRequestRespond类
CKCRequestRespond::CKCRequestRespond(IWebRequestRespond& re, IKCWebWork& ww)
    : m_re(re), m_webWork(ww)
{
    // 开始日志
    const char *pCookie = m_re.GetCookieStr(),
               *pGet = m_re.GetGetArgStr();
    m_webWork.WriteLogInfo((string(m_webWork.getHint("Reander_page_")) + m_re.GetUriFilename()).c_str(), __FUNCTION__,
                           (lexical_cast<string>(reinterpret_cast<unsigned int>(this)) + " - " + lexical_cast<string>(pthread_self()) + " - " + m_re.GetClientIP() +
                           (nullptr != pGet ? string("  -  GET: ") + pGet : string("")) +
                           (nullptr != pCookie ? string("  -  Cookie: ") + pCookie : string(""))
                           ).c_str());
}

CKCRequestRespond::~CKCRequestRespond()
{
    // 结束日志
    m_webWork.WriteLogInfo((string(m_webWork.getHint("Reander_page_end_")) + m_re.GetUriFilename()).c_str(), __FUNCTION__,
                        (lexical_cast<string>(reinterpret_cast<unsigned int>(this)) + " - " + m_re.GetClientIP()).c_str());
    // 资源释放
    this->Release();
}

// 服务接口
IKCWebWork& CKCRequestRespond::GetWork(void)
{
    return m_webWork;
}

// 释放
void CKCRequestRespond::Release(void)
{
    try
    {
        // 释放缓冲数据
        auto TryFunc = [&]()
        {
            TReStepDataPtrList sdFreeList;
            sdFreeList.swap(m_ReStepDataPtrList);
            m_ReStepDataPtrList.clear();
            sdFreeList.clear();
        };
        // 信号捕捉
        TSignoTry<decltype(TryFunc)> sigTry(TryFunc, *this, (string("Release(") + this->GetLocalFilename() + ")").c_str());
        this->SignoCatch().SignoCatchPoint(sigTry);
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = this->GetLocalFilename();
        TLogInfo log(ex);
        log.m_place = __FUNCTION__;
        m_webWork.WriteLog(log);
    }
    catch(...)
    {
        m_webWork.WriteLogError(m_webWork.getHint("Unknown_exception"), __FUNCTION__, this->GetLocalFilename());
    }
}

// 提交
void CKCRequestRespond::CommitResponseBody(void)
{
    m_re.AddResponseHeader("Server-Expand", c_KnewcodeVersion);
    auto iter = m_ResponseBodys.find(m_re.GetResponseContentType());
    if (m_ResponseBodys.end() != iter)
        m_re.AddResponseBody(iter->second.c_str(), iter->second.size());
}

///////////////////////////////////////////////////////////////
// 信号处理实例接口
ISignoCatch& CKCRequestRespond::SignoCatch(void)
{
    return m_webWork.getContext().SignoCatch();
}

// 锁
IKcLockWork& CKCRequestRespond::LockWork(void)
{
    return m_webWork.getContext().LockWork();
}

// 超时时间
int CKCRequestRespond::GetTimeOutSeconds(void) const
{
    return m_webWork.getContext().GetTimeOutSeconds();
}

// 执行中的页面数据接口
IActionData* CKCRequestRespond::GetActionData(void)
{
    return m_ActionData;
}

void CKCRequestRespond::SetActionData(IActionData* act)
{
    m_ActionData = act;
}

////////////////////////////请求部分///////////////////////////
// 得到服务器端全部信息
const char* CKCRequestRespond::GetAllInfo(const char* endTag)
{
    return m_re.GetAllInfo(endTag);
}

// 得到本地完整文件名
const char* CKCRequestRespond::GetLocalFilename(void)
{
    m_webWork.WriteLogTrace((string(m_webWork.getHint("Get_full_name_of_the_local_file_")) + m_re.GetUriFilename()).c_str(), __FUNCTION__, m_re.GetLocalFilename());
    if (!filesystem::exists(m_re.GetLocalFilename()))
        throw TKCWebWorkException(0, __FUNCTION__, string(m_webWork.getHint("Don_t_exists_file_")) + m_re.GetLocalFilename(), m_webWork);
    return m_re.GetLocalFilename();
}

// 得到网络文件名
const char* CKCRequestRespond::GetUriFilename(void)
{
    return m_re.GetUriFilename();
}

// 协议
const char* CKCRequestRespond::GetProtocol(void)
{
    m_Protocol = m_re.GetProtocol();
    vector<string> vecSegTag;
    boost::split(vecSegTag, m_Protocol, boost::is_any_of("/"));
    if (!vecSegTag.empty()) m_Protocol = vecSegTag[0];
    return m_Protocol.c_str();
}

// 主机名
const char* CKCRequestRespond::GetHostName(void)
{
    return m_re.GetHostName();
}

// 端口
int CKCRequestRespond::GetPort(void)
{
    return m_re.GetPort();
}

// 客户端信息
const char* CKCRequestRespond::GetUserAgent(void)
{
    return m_re.GetUserAgent();
}

// 客户端IP
const char* CKCRequestRespond::GetClientIP(void)
{
    return m_re.GetClientIP();
}

// 接收的信息
const char* CKCRequestRespond::GetAccept(const char* name)
{
    return m_re.GetAccept(name);
}

// GET的参数
const char* CKCRequestRespond::GetGetArgStr(void)
{
    return m_re.GetGetArgStr();
}

// Post的参数
const char* CKCRequestRespond::GetPostArgType(void)
{
    return m_re.GetPostArgType();
}
int CKCRequestRespond::GetPostArgLength(void)
{
    return m_re.GetPostArgLength();
}
int CKCRequestRespond::GetPostArgBuffer(char* buf, int len)
{
    return m_re.GetPostArgBuffer(buf, len);
}

// 得到网站本地完整根目录
const char* CKCRequestRespond::GetLocalRootPath(void)
{
    return m_re.GetLocalRootPath();
}

// 得到当前页本地完整根目录
const char* CKCRequestRespond::GetLocalPagePath(void)
{
    filesystem::path _path(m_re.GetLocalFilename());
    m_LocalPagePath = _path.parent_path().string();
    m_LocalPagePath = CUtilFunc::FormatPath(m_LocalPagePath);
    return m_LocalPagePath.c_str();
}

// 按名称得到请求头
const char* CKCRequestRespond::GetRequestHeader(const char* name)
{
    return m_re.GetRequestHeader(name);
}

// 得到Cookie
const char* CKCRequestRespond::GetCookieStr(void)
{
    return m_re.GetCookieStr();
}

////////////////////////////应答部分///////////////////////////
// 响应文本类型
bool CKCRequestRespond::SetResponseContentType(const char* ct)
{
    return m_re.SetResponseContentType(dynamic_cast<CKCWebWork&>(m_webWork).FixContentTypeString(ct));
}
const char* CKCRequestRespond::GetResponseContentType(void)
{
    return m_re.GetResponseContentType();
}

// 输出网页内容
bool CKCRequestRespond::AddResponseBody(const char* buf, int nbyte)
{
    return m_re.AddResponseBody(buf, nbyte);
}

// 按名称添加响应头
void CKCRequestRespond::AddResponseHeader(const char* name, const char* val)
{
    m_re.AddResponseHeader(name, val);
}

// 添加Cookie
bool CKCRequestRespond::AddCookie(const char* name, const char* val, const char* expires, const char* path, const char* domain)
{
    string  sName = nullptr != name ? name : "",
            sVal = nullptr != val ? val : "",
            sExpires = nullptr != expires ? expires : "",
            sPath = nullptr != path ? path : "",
            sDomain = nullptr != domain ? domain : "";
    bool bResult = !sName.empty();
    if (bResult)
    {
        string sCookie = sName + "=" + sVal;
        if (!sExpires.empty())
            sCookie += "; expires=" + CUtilFunc::ISO2GMT(sExpires);
        if (!sPath.empty())
            sCookie += "; path=" + sPath;
        if (!sDomain.empty())
            sCookie += "; domain=" + sDomain;
        m_re.AddResponseHeader("Set-Cookie", sCookie.c_str());
    }
    return bResult;
}

// 输出网页内容：参数为输出内容
bool CKCRequestRespond::OutputData(const char* ct, const char* buf, int nbyte)
{
    m_webWork.WriteLogTrace((string("") + "[" + ct + "]<" + m_re.GetUriFilename() + "><" + m_re.GetLocalFilename() + ">").c_str(), __FUNCTION__, buf);
    if (nullptr != buf && nullptr != ct && nbyte > 0) m_ResponseBodys[ct].append(buf, nbyte);
    else if (nullptr != buf && nullptr != ct) m_ResponseBodys[ct] += buf;
    return true;
}
bool CKCRequestRespond::OutputHTML(const char* html)
{
    return this->OutputData("text/html", html);
}

////////////////////////请求过程中的数据///////////////////////
// 判断某名称数据是否存在
bool CKCRequestRespond::ExistsReStepData(const char* name)
{
    return m_ReStepDataPtrList.end() != m_ReStepDataPtrList.find(name);
}

// 得到某名称数据
IKCRequestRespond::IReStepData& CKCRequestRespond::GetReStepData(const char* name)
{
    auto iter = m_ReStepDataPtrList.find(name);
    if (m_ReStepDataPtrList.end() == iter)
        throw TKCWebWorkException(0, __FUNCTION__, string(m_webWork.getHint("Don_t_exists_data_")) + name, m_webWork);
    return *iter->second;
}

// 添加某名称数据
bool CKCRequestRespond::AddReStepData(const char* name, IKCRequestRespond::IReStepData*& data)
{
    if (nullptr == data)
        throw TKCWebWorkException(0, __FUNCTION__, string(m_webWork.getHint("Null_Point_Data_")) + name, m_webWork);
    if (!ExistsReStepData(name))
    {
        m_ReStepDataPtrList.insert(make_pair(string(name), TReStepDataPtr(data)));
        if (m_webWork.getContext().GetCfgLogLevel() <= (int)log::trivial::debug)
            m_webWork.WriteLogDebug((string(m_webWork.getHint("Register_Data_")) + name).c_str(), __FUNCTION__, typeid(*data).name());
    }
    else
    {
        delete data;
        data = nullptr;
        throw TKCWebWorkException(0, __FUNCTION__, string(m_webWork.getHint("Repeat_Register_Data_")) + name, m_webWork);
    }
    return ExistsReStepData(name);
}
