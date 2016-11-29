#include "std.h"
#include "request_respond.h"
#include "apache_work.h"

////////////////////////////////////////////////////////////////////////////////
// CWebRequestRespond类
KC::CWebRequestRespond::CWebRequestRespond(CAPACHEWork& Parent, request_rec& r, string pgPath)
    : m_Parent(Parent), m_r(r), m_PgPath(pgPath)
{
}

// 得到服务器端全部信息
const char* KC::CWebRequestRespond::GetAllInfo(const char* endTag)
{
    m_AllInfo = string(c_KnewcodeVersion) + endTag;
    m_AllInfo += "DocumentRoot: " + string(this->GetLocalRootPath()) + endTag;
    m_AllInfo += "filename: " + CUtilFunc::PCharSafeToStr(m_r.filename) + endTag;
    m_AllInfo += "uri: " + CUtilFunc::PCharSafeToStr(m_r.uri) + endTag;
    m_AllInfo += "canonical_filename: " + CUtilFunc::PCharSafeToStr(m_r.canonical_filename) + endTag;
    m_AllInfo += "unparsed_uri: " + CUtilFunc::PCharSafeToStr(m_r.unparsed_uri) + endTag;
    m_AllInfo += "hostname: " + CUtilFunc::PCharSafeToStr(m_r.hostname) + endTag;
    m_AllInfo += "protocol: " + CUtilFunc::PCharSafeToStr(m_r.protocol) + endTag;
    m_AllInfo += "proto_num: " + lexical_cast<string>(m_r.proto_num) + endTag;
    m_AllInfo += "ap_auth_type: " + CUtilFunc::PCharSafeToStr(m_r.ap_auth_type) + endTag;
    m_AllInfo += "user: " + CUtilFunc::PCharSafeToStr(m_r.user) + endTag;
    m_AllInfo += "vlist_validator: " + CUtilFunc::PCharSafeToStr(m_r.vlist_validator) + endTag;
    m_AllInfo += "content_encoding: " + CUtilFunc::PCharSafeToStr(m_r.content_encoding) + endTag;
    m_AllInfo += "handler: " + CUtilFunc::PCharSafeToStr(m_r.handler) + endTag;
    m_AllInfo += "content_type: " + CUtilFunc::PCharSafeToStr(m_r.content_type) + endTag;
    m_AllInfo += "range: " + CUtilFunc::PCharSafeToStr(m_r.range) + endTag;
    m_AllInfo += "status_line: " + CUtilFunc::PCharSafeToStr(m_r.status_line) + endTag;
    m_AllInfo += "method: " + CUtilFunc::PCharSafeToStr(m_r.method) + endTag;
    m_AllInfo += "the_request: " + CUtilFunc::PCharSafeToStr(m_r.the_request) + endTag;
    m_AllInfo += "args: " + CUtilFunc::PCharSafeToStr(m_r.args) + endTag;
    m_AllInfo += "finfo.fname: " + CUtilFunc::PCharSafeToStr(m_r.finfo.fname) + endTag;
    m_AllInfo += "finfo.name: " + CUtilFunc::PCharSafeToStr(m_r.finfo.name) + endTag;
    m_AllInfo += "parsed_uri.scheme: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.scheme) + endTag;
    m_AllInfo += "parsed_uri.hostinfo: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.hostinfo) + endTag;
    m_AllInfo += "parsed_uri.user: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.user) + endTag;
    m_AllInfo += "parsed_uri.password: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.password) + endTag;
    m_AllInfo += "parsed_uri.hostname: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.hostname) + endTag;
    m_AllInfo += "parsed_uri.port_str: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.port_str) + endTag;
    m_AllInfo += "parsed_uri.path: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.path) + endTag;
    m_AllInfo += "parsed_uri.query: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.query) + endTag;
    m_AllInfo += "parsed_uri.fragment: " + CUtilFunc::PCharSafeToStr(m_r.parsed_uri.fragment) + endTag;
    m_AllInfo += "path_info: " + CUtilFunc::PCharSafeToStr(m_r.path_info) + endTag;
    if (nullptr != m_r.server)
    {
        m_AllInfo += "server->defn_name: " + CUtilFunc::PCharSafeToStr(m_r.server->defn_name) + endTag;
        m_AllInfo += "server->path: " + CUtilFunc::PCharSafeToStr(m_r.server->path) + endTag;
        m_AllInfo += "server->server_admin: " + CUtilFunc::PCharSafeToStr(m_r.server->server_admin) + endTag;
        m_AllInfo += "server->server_hostname: " + CUtilFunc::PCharSafeToStr(m_r.server->server_hostname) + endTag;
        m_AllInfo += "server->server_scheme: " + CUtilFunc::PCharSafeToStr(m_r.server->server_scheme) + endTag;
    }
    if (nullptr != m_r.connection)
    {
        m_AllInfo += "connection->remote_ip: " + CUtilFunc::PCharSafeToStr(this->GetClientIP()) + endTag;
        m_AllInfo += "connection->remote_host: " + CUtilFunc::PCharSafeToStr(m_r.connection->remote_host) + endTag;
        m_AllInfo += "connection->remote_logname: " + CUtilFunc::PCharSafeToStr(m_r.connection->remote_logname) + endTag;
        m_AllInfo += "connection->local_ip: " + CUtilFunc::PCharSafeToStr(m_r.connection->local_ip) + endTag;
        m_AllInfo += "connection->local_host: " + CUtilFunc::PCharSafeToStr(m_r.connection->local_host) + endTag;
    }
    m_AllInfo += "User-Agent: " + CUtilFunc::PCharSafeToStr(apr_table_get(m_r.headers_in, "User-Agent")) + endTag;
    //string sPostType = this->GetPostArgType();
    //string sPostStr = this->GetPostArgStr();
    //m_AllInfo += "POST: " + sPostType + endTag + sPostStr + endTag;
    return m_AllInfo.c_str();
}

////////////////////////////请求部分///////////////////////////
// 得到本地完整文件名
const char* KC::CWebRequestRespond::GetLocalFilename(void)
{
    return m_r.filename;
}

// 得到网络文件名
const char* KC::CWebRequestRespond::GetUriFilename(void)
{
    return m_r.uri;
}

// 协议
const char* KC::CWebRequestRespond::GetProtocol(void)
{
    return m_r.protocol;
}

// 主机名
const char* KC::CWebRequestRespond::GetHostName(void)
{
    return m_r.hostname;
}

// 端口
int KC::CWebRequestRespond::GetPort(void)
{
    int result = 80;
    try
    {
        result = lexical_cast<int>(m_r.parsed_uri.port_str);
    }
    catch(...)
    {
    }
    return result;
}

// 客户端信息
const char* KC::CWebRequestRespond::GetUserAgent(void)
{
    return apr_table_get(m_r.headers_in, "User-Agent");
}

// 客户端IP
const char* KC::CWebRequestRespond::GetClientIP(void)
{
#ifdef WIN32    // Windows环境
    return m_r.connection->remote_ip;
#else           // linux环境
    return m_r.connection->client_ip;
#endif
}

// 接收的信息
const char* KC::CWebRequestRespond::GetAccept(const char* name)
{
    if (CUtilFunc::StrToLower(name) == "encoding")
        return apr_table_get(m_r.headers_in, "Accept-Encoding");
    else if (CUtilFunc::StrToLower(name) == "language")
        return apr_table_get(m_r.headers_in, "Accept-Language");
    else
        return apr_table_get(m_r.headers_in, "Accept");
}

// GET的参数
const char* KC::CWebRequestRespond::GetGetArgStr(void)
{
    return m_r.args;
}

// Post的参数
const char* KC::CWebRequestRespond::GetPostArgType(void)
{
    if(M_POST == m_r.method_number)
        return apr_table_get(m_r.headers_in, "Content-Type");
    return "";
}
int KC::CWebRequestRespond::GetPostArgLength(void)
{
    if(M_POST == m_r.method_number)
        return atoi(apr_table_get(m_r.headers_in, "Content-Length"));
    return 0;
}
int KC::CWebRequestRespond::GetPostArgBuffer(char* buf, int len)
{
    memset(buf, 0, len);
    int iResult = 0;
    if(M_POST == m_r.method_number && OK == ap_setup_client_block(&m_r, REQUEST_CHUNKED_DECHUNK) && 1 == ap_should_client_block(&m_r))
        for (int i = 0; iResult < len && (i = ap_get_client_block(&m_r, buf + iResult, len - iResult)) > 0; iResult += i);
    return iResult;
}

// 得到网站本地完整根目录
const char* KC::CWebRequestRespond::GetLocalRootPath(void)
{
    return m_PgPath.c_str();
}

// 按名称得到请求头
const char* KC::CWebRequestRespond::GetRequestHeader(const char* name)
{
    return apr_table_get(m_r.headers_in, name);
}

// 得到Cookie
const char* KC::CWebRequestRespond::GetCookieStr(void)
{
    return this->GetRequestHeader("Cookie");
}

////////////////////////////应答部分///////////////////////////
// 设置响应文本类型
bool KC::CWebRequestRespond::SetResponseContentType(const char* ct)
{
    m_r.content_type = ct;
    return true;
}
const char* KC::CWebRequestRespond::GetResponseContentType(void)
{
    return m_r.content_type;
}

// 输出网页内容
bool KC::CWebRequestRespond::AddResponseBody(const char* buf, int nbyte)
{
    bool bResult = false;
    if (nullptr != buf &&nbyte > 0)
        bResult = ap_rwrite(buf, nbyte, &m_r) > 0;
    else if (nullptr != buf)
        bResult = ap_rputs(buf, &m_r) > 0;
    return bResult;
}

// 按名称添加响应头
void KC::CWebRequestRespond::AddResponseHeader(const char* name, const char* val)
{
    apr_table_set(m_r.headers_out, name, val);
}
