#include "request_respond_parm.h"

////////////////////////////////////////////////////////////////////////////////
// CKCCtrlParmInOut 类
CKCCtrlParmInOut::CKCCtrlParmInOut(ICoreParmJson& own, string nameErrCode, string nameErrMsg, string charset, bool jsonCaseSensitive)
    : m_nameErrCode(nameErrCode), m_nameErrMsg(nameErrMsg), m_charset(charset), m_jsonCaseSensitive(jsonCaseSensitive)
    , m_own(own), m_isJson(true), m_jsonContentType(c_DefaultResponseContentType + string(";charset=") +  m_charset), m_contentType(m_jsonContentType)
{
}

// 参数类型是否json
void CKCCtrlParmInOut::SetIsJson(void)
{
    m_isJson = true;
    m_contentType = m_jsonContentType;
}
bool CKCCtrlParmInOut::IsJson(void) const
{
    return m_isJson;
}

// 正文类型
void CKCCtrlParmInOut::SetContentType(const char* contentType)
{
    m_isJson = false;
    m_contentType = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(contentType));
    if (m_contentType.empty()) SetIsJson();
}
const char* CKCCtrlParmInOut::GetContentType(void) const
{
    return m_contentType.c_str();
}

// 头部数据（针对web请求的应答）
void CKCCtrlParmInOut::SetHeader(const char* key, const char* val)
{
    string sKey = CUtilFunc::PCharSafeToStr(key);
    string sVal = CUtilFunc::PCharSafeToStr(val);
    auto it = m_header.find(sKey);
    if (m_header.end() != it) it->second = sVal;
    else m_header.insert(std::make_pair(sKey, sVal));
}
const char* CKCCtrlParmInOut::GetHeader(const char* key, const char* def) const
{
    string sKey = CUtilFunc::PCharSafeToStr(key);
    auto it = m_header.find(sKey);
    if (m_header.end() != it) return it->second.c_str();
    return def;
}

// 内容数据
void CKCCtrlParmInOut::SetContent(const char* data, const unsigned len)
{
    m_content.clear();
    m_content.append(data, len);
}
const char* CKCCtrlParmInOut::GetContent(void) const
{
    return m_content.c_str();
}
unsigned CKCCtrlParmInOut::GetContentLength(void) const
{
    return static_cast<unsigned>(m_content.size());
}

// 是否区分大小写
bool CKCCtrlParmInOut::JsonCaseSensitive(void) const
{
    return m_jsonCaseSensitive;
}
// 字符集
const char* CKCCtrlParmInOut::GetCharset(void) const
{
    return m_charset.c_str();
}

// 源最终的错误码和错误信息
int CKCCtrlParmInOut::GetErrCode(void) const
{
    return m_own.Parm().IsJson() ? static_cast<int>(m_own.Json().GetVal(m_nameErrCode.c_str(), 0, m_jsonCaseSensitive)) : 0;
}
const char* CKCCtrlParmInOut::GetErrMsg(void) const
{
    static thread_local string sStr;
    sStr = m_own.Parm().IsJson() ? m_own.Json().GetStr(m_nameErrMsg.c_str(), "", m_jsonCaseSensitive) : "";
    // if (m_charset == c_RESTful_GBK) sStr = CUtilFunc::Utf8ToGbk(sStr);
    return sStr.c_str();
}
