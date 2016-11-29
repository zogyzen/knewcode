#include "std.h"
#include "request_respond.h"
#include "web_work.h"

////////////////////////////////////////////////////////////////////////////////
// CWebRequestRespond类
KC::CWebRequestRespond::CWebRequestRespond(CWebWork& Parent, const char* request, const char* path)
    : m_Parent(Parent), m_request(request), m_path(path)
{
}


// 得到服务器端全部信息
const char* KC::CWebRequestRespond::GetAllInfo(const char* endTag)
{
    return "";
}

////////////////////////////请求部分///////////////////////////
// 得到本地完整文件名
const char* KC::CWebRequestRespond::GetLocalFilename(void)
{
#ifdef WIN32    // Windows环境
    return "D:\\mycode\\KCPage\\index.kc";
#else           // linux环境
    return "/home/zogy/kc_pages/index.kc";
#endif
}

// 得到网络文件名
const char* KC::CWebRequestRespond::GetUriFilename(void)
{
    return "/index.kc";
}

// 协议
const char* KC::CWebRequestRespond::GetProtocol(void)
{
    return "http";
}

// 主机名
const char* KC::CWebRequestRespond::GetHostName(void)
{
    return "127.0.0.1";
}

// 端口
int KC::CWebRequestRespond::GetPort(void)
{
    return 9000;
}

// 客户端信息
const char* KC::CWebRequestRespond::GetUserAgent(void)
{
    return "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.124 Safari/537.36";
}

// 客户端IP
const char* KC::CWebRequestRespond::GetClientIP(void)
{
    return "127.0.0.1";
}

// 接收的信息
const char* KC::CWebRequestRespond::GetAccept(const char* name)
{
    if (CUtilFunc::StrToLower(name) == "encoding")
        return "gzip,deflate";
    else if (CUtilFunc::StrToLower(name) == "language")
        return "zh-CN,zh;q=0.8";
    else
        return "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
}

// GET的参数
const char* KC::CWebRequestRespond::GetGetArgStr(void)
{
    return "a=1&b=2";
}

// Post的参数
const char* KC::CWebRequestRespond::GetPostArgType(void)
{
    return "application/x-www-form-urlencoded";
}
int KC::CWebRequestRespond::GetPostArgLength(void)
{
    return 7;
}
int KC::CWebRequestRespond::GetPostArgBuffer(char* buf, int len)
{
    memcpy(buf, "c=3&d=4", min(len, 7));
    return min(len, 7);
}

// 得到网站本地完整根目录
const char* KC::CWebRequestRespond::GetLocalRootPath(void)
{
#ifdef WIN32    // Windows环境
    return "D:\\mycode\\KCPage";
#else           // linux环境
    return "/home/zogy/kc_pages";
#endif
}

// 按名称得到请求头
const char* KC::CWebRequestRespond::GetRequestHeader(const char*)
{
    return "";
}

// 得到Cookie
const char* KC::CWebRequestRespond::GetCookieStr(void)
{
    return "LoginName=admin; AdminPWD=outreach; CustLoginName=HR; CustPWD=123";
}

////////////////////////////应答部分///////////////////////////
// 响应文本类型
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
bool KC::CWebRequestRespond::AddResponseBody(const char* html, int)
{
    m_response += html;
    return false;
}

// 按名称添加响应头
void KC::CWebRequestRespond::AddResponseHeader(const char*, const char*)
{
}
