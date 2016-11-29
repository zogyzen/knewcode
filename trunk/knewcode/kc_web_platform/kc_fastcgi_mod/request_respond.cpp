#include "std.h"
#include "request_respond.h"
#include "fastcgi_work.h"

////////////////////////////////////////////////////////////////////////////////
// CWebRequestRespond类
KC::CWebRequestRespond::CWebRequestRespond(CFastCGIWork& Parent, FCGX_Request& r) : m_Parent(Parent), m_re(r)
{
}

// 得到服务器端全部信息
const char* KC::CWebRequestRespond::GetAllInfo(const char* endTag)
{
    m_AllInfo = string(c_KnewcodeVersion) + endTag;
    for (char** env = m_re.envp; nullptr != *env; ++env)
        m_AllInfo += string(*env) + endTag;
    //string sPostStr = this->GetPostArgStr();
    //string sPostType = this->GetPostArgType();
    //m_AllInfo += "POST: " + sPostType + endTag + sPostStr + endTag;
    return m_AllInfo.c_str();
}

////////////////////////////请求部分///////////////////////////
// 得到本地完整文件名
const char* KC::CWebRequestRespond::GetLocalFilename(void)
{
    return FCGX_GetParam("PATH_TRANSLATED", m_re.envp);
}

// 得到网络文件名
const char* KC::CWebRequestRespond::GetUriFilename(void)
{
    return FCGX_GetParam("URL", m_re.envp);
}

// 协议
const char* KC::CWebRequestRespond::GetProtocol(void)
{
    return FCGX_GetParam("SERVER_PROTOCOL", m_re.envp);
}

// 主机名
const char* KC::CWebRequestRespond::GetHostName(void)
{
    return FCGX_GetParam("SERVER_NAME", m_re.envp);
}

// 端口
int KC::CWebRequestRespond::GetPort(void)
{
    int result = 80;
    try
    {
        result = lexical_cast<int>(FCGX_GetParam("SERVER_PORT", m_re.envp));
    }
    catch(...)
    {
    }
    return result;
}

// 客户端信息
const char* KC::CWebRequestRespond::GetUserAgent(void)
{
    return FCGX_GetParam("HTTP_USER_AGENT", m_re.envp);
}

// 客户端IP
const char* KC::CWebRequestRespond::GetClientIP(void)
{
    return FCGX_GetParam("REMOTE_ADDR", m_re.envp);
}

// 接收的信息
const char* KC::CWebRequestRespond::GetAccept(const char* name)
{
    if (CUtilFunc::StrToLower(name) == "encoding")
        return FCGX_GetParam("HTTP_ACCEPT_ENCODING", m_re.envp);
    else if (CUtilFunc::StrToLower(name) == "language")
        return FCGX_GetParam("HTTP_ACCEPT_LANGUAGE", m_re.envp);
    else
        return FCGX_GetParam("HTTP_ACCEPT", m_re.envp);
}

// GET的参数
const char* KC::CWebRequestRespond::GetGetArgStr(void)
{
    return FCGX_GetParam("QUERY_STRING", m_re.envp);
}

// Post的参数
const char* KC::CWebRequestRespond::GetPostArgType(void)
{
    if(CUtilFunc::StrToLower(FCGX_GetParam("REQUEST_METHOD", m_re.envp)) == "post")
        return FCGX_GetParam("CONTENT_TYPE", m_re.envp);
    return "";
}
int KC::CWebRequestRespond::GetPostArgLength(void)
{
    if(CUtilFunc::StrToLower(FCGX_GetParam("REQUEST_METHOD", m_re.envp)) == "post")
        return atoi(FCGX_GetParam("CONTENT_LENGTH", m_re.envp));
    return 0;
}
int KC::CWebRequestRespond::GetPostArgBuffer(char* buf, int len)
{
    memset(buf, 0, len);
    if(CUtilFunc::StrToLower(FCGX_GetParam("REQUEST_METHOD", m_re.envp)) == "post")
        return FCGX_GetStr(buf, len, m_re.in);
    return 0;
}

// 得到网站本地完整根目录
const char* KC::CWebRequestRespond::GetLocalRootPath(void)
{
    return FCGX_GetParam("DOCUMENT_ROOT", m_re.envp);
}

// 按名称得到请求头
const char* KC::CWebRequestRespond::GetRequestHeader(const char* name)
{
    return FCGX_GetParam(name, m_re.envp);
}

// 得到Cookie
const char* KC::CWebRequestRespond::GetCookieStr(void)
{
    return this->GetRequestHeader("HTTP_COOKIE");
}

////////////////////////////应答部分///////////////////////////
// 设置响应文本类型
bool KC::CWebRequestRespond::SetResponseContentType(const char* ct)
{
    m_ContentType = ct;
    return true;
}
const char* KC::CWebRequestRespond::GetResponseContentType(void)
{
    return m_ContentType.c_str();
}

// 输出网页内容
bool KC::CWebRequestRespond::AddResponseBody(const char* buf, int nbyte)
{
    if (nullptr != buf && nbyte > 0) m_ResponseBody.append(buf, nbyte);
    else if (nullptr != buf) m_ResponseBody += buf;
    return true;
}

// 按名称添加响应头
void KC::CWebRequestRespond::AddResponseHeader(const char* name, const char* val)
{
    m_ResponseHead += string(name) + ":" + val + "\r\n";
}

///////////////////////////////////////////////////////////////
// 提交应答
void KC::CWebRequestRespond::CommitResponse(void)
{
    // 输出页头
    FCGX_FPrintF(m_re.out, ("Content-type: " + m_ContentType + "\r\n").c_str());
    FCGX_FPrintF(m_re.out, m_ResponseHead.c_str());
    FCGX_FPrintF(m_re.out, "\r\n");
    // 输出页内容
    FCGX_PutStr(m_ResponseBody.c_str(), m_ResponseBody.size(), m_re.out);
    FCGX_Finish_r(&m_re);
}
