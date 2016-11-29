#include "std.h"
#include "request_respond.h"
#include "isapi_work.h"

////////////////////////////////////////////////////////////////////////////////
// CKCRespond类
KC::CKCRequestRespond::CKCRequestRespond(CISAPIWork& Parent, EXTENSION_CONTROL_BLOCK& ECB)
    : m_Parent(Parent), m_ECB(ECB)
{
}

// 得到服务器端全部信息
string KC::CKCRequestRespond::GetAllInfo(const char* endTag)
{
    string sResult = "";
    sResult += "lpszPathTranslated: " + CUtilFunc::PCharSafeToStr(m_ECB.lpszPathTranslated) + endTag;
    sResult += "lpbData: " + CUtilFunc::PCharSafeToStr((char*)m_ECB.lpbData) + endTag;
    sResult += "lpszLogData: " + CUtilFunc::PCharSafeToStr(m_ECB.lpszLogData) + endTag;
    sResult += "lpszMethod: " + CUtilFunc::PCharSafeToStr(m_ECB.lpszMethod) + endTag;
    sResult += "lpszQueryString: " + CUtilFunc::PCharSafeToStr(m_ECB.lpszQueryString) + endTag;
    sResult += "lpszPathInfo: " + CUtilFunc::PCharSafeToStr(m_ECB.lpszPathInfo) + endTag;
    sResult += "lpszContentType: " + CUtilFunc::PCharSafeToStr(m_ECB.lpszContentType) + endTag;
    return sResult;
}

////////////////////////////请求部分///////////////////////////
// 得到本地完整文件名
string KC::CKCRequestRespond::GetLocalFilename(void)
{
    return m_ECB.lpszPathTranslated;
}

// 得到网络文件名
string KC::CKCRequestRespond::GetUriFilename(void)
{
    return m_ECB.lpszPathInfo;
}

// 协议
string KC::CKCRequestRespond::GetProtocol(void)
{
    return "http";
}

// 主机名
string KC::CKCRequestRespond::GetHostName(void)
{
    return "127.0.0.1";
}

// 端口
int KC::CKCRequestRespond::GetPort(void)
{
    return 9000;
}

// GET的参数
string KC::CKCRequestRespond::GetGetArgStr(void)
{
    return "a=1&b=2";
}

// 得到网站本地完整根目录
string KC::CKCRequestRespond::GetLocalRootPath(void)
{
    return "C:/KCPage";
}

////////////////////////////应答部分///////////////////////////
// 输出网页内容
bool KC::CKCRequestRespond::OutputHTML(const char* html)
{
    DWORD dwSize = strlen(html);
    return m_ECB.WriteClient(m_ECB.ConnID, (char*)html, &dwSize, 0);
}
