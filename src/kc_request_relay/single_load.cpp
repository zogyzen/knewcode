#include "single_load.h"
#include "ctrl_request.h"

// 引出函数
extern "C"
{
    // 创建实例
    ISLWebRequest& create(IKCSingleLoadSoCB& cb)
    {
        return *new CSLWebRequest(cb);
    }
    // 销毁实例
    void destroy(ISLWebRequest& wr)
    {
        delete &wr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// CSLWebRequest 类
CSLWebRequest::CSLWebRequest(IKCSingleLoadSoCB& cb) : m_cb(cb)
{
}

CSLWebRequest::~CSLWebRequest(void)
{
}

// POST请求
unsigned CSLWebRequest::POST(const char*& respond, const char* srv, const char* pth, const char* prm, const char* body, int len, const char* cookie)
{
    string sBody;
    int iRealLen = nullptr != body && len > 0 ? len : (nullptr != body ? static_cast<int>(strlen(body)) : 0);
    if (iRealLen > 0) sBody.append(body, iRealLen);
    auto fExceptInfo = [&](void)
    {
        return (boost::format("Post-> %s%s%s\n%s") % srv % pth % prm % sBody.substr(0, 500)).str();
    };
    try
    {
        string sSrv = CUtilFunc::PCharSafeToStr(srv), sPth = CUtilFunc::PCharSafeToStr(pth), sPrm = CUtilFunc::PCharSafeToStr(prm);
        string sClnCookies = CUtilFunc::PCharSafeToStr(cookie);
        KcHeaders headers;
        CCtrlRelay::MakeCookie(sSrv, sPth, sClnCookies, headers, m_cookieMan);
        static thread_local string sRespond;
        sRespond = _PostGet(*this, headers, sSrv, sPth, sPrm, sBody, true);
        respond = sRespond.c_str();
        return static_cast<unsigned>(sRespond.size());
    }
    catch (std::exception &ex)
    {
        m_cb.WriteLogError(ex.what(), __CURR_CODE_PLACE_C__, fExceptInfo().c_str());
    }
    catch (...)
    {
        m_cb.WriteLogError("Unknown Exception", __CURR_CODE_PLACE_C__, fExceptInfo().c_str());
    }
    return 0;
}

// GET请求
unsigned CSLWebRequest::GET(const char*& respond, const char* srv, const char* pth, const char* prm, const char* cookie)
{
    auto fExceptInfo = [&](void)
    {
        return (boost::format("Get-> %s%s%s") % srv % pth % prm).str();
    };
    try
    {
        string sSrv = CUtilFunc::PCharSafeToStr(srv), sPth = CUtilFunc::PCharSafeToStr(pth), sPrm = CUtilFunc::PCharSafeToStr(prm);
        string sClnCookies = CUtilFunc::PCharSafeToStr(cookie);
        KcHeaders headers;
        CCtrlRelay::MakeCookie(sSrv, sPth, sClnCookies, headers, m_cookieMan);
        static thread_local string sRespond;
        sRespond = _PostGet(*this, headers, sSrv, sPth, sPrm, "", false);
        respond = sRespond.c_str();
        return static_cast<unsigned>(sRespond.size());
    }
    catch (std::exception &ex)
    {
        m_cb.WriteLogError(ex.what(), __CURR_CODE_PLACE_C__, fExceptInfo().c_str());
    }
    catch (...)
    {
        m_cb.WriteLogError("Unknown Exception", __CURR_CODE_PLACE_C__, fExceptInfo().c_str());
    }
    return 0;
}

// 得到cookie
const char* CSLWebRequest::Cookie(const char* srv, const char* cookie)
{
    const char* pResult = nullptr;
    auto ckLs = m_cookieMan.getCookieList(CUtilFunc::PCharSafeToStr(srv));
    if (ckLs.get() != nullptr)
    {
        auto ck = ckLs->getCookie(CUtilFunc::PCharSafeToStr(cookie));
        if (ck.get() != nullptr)
            pResult = ck->m_val.c_str();
    }
    return pResult;
}
const char* CSLWebRequest::Cookie(const char* srv)
{
    const char* pResult = nullptr;
    auto ckLs = m_cookieMan.getCookieList(CUtilFunc::PCharSafeToStr(srv));
    if (ckLs.get() != nullptr)
    {
        static thread_local string sCookie;
        sCookie = ckLs->makeCookie("");
        pResult = sCookie.c_str();
    }
    return pResult;
}

// 得到响应头
const char* CSLWebRequest::RespondHeader(const char* key, const unsigned pos)
{
    boost::shared_lock<boost::shared_mutex> lck(m_mtxresh);
    auto its = m_respondHeader.equal_range(key);
    auto it = its.first;
    for (unsigned i = 0; its.second != it && i < pos; ++it, ++i);
    if (its.second != it) return its.first->second.c_str();
    else return nullptr;
}

// 转换状态信息
const char* CSLWebRequest::status_message(int status)
{
    return httplib::detail::status_message(status);
    // return httplib::status_message(status);
}
// 响应头
void CSLWebRequest::RespondHeader(string sSrv, const KcHeaders& h)
{
    CCtrlRelay::ParseCookie(sSrv, h, m_cookieMan);
    boost::unique_lock<boost::shared_mutex> lck(m_mtxresh);
    m_respondHeader.swap(const_cast<KcHeaders&>(h));
}
// 日志
bool CSLWebRequest::WriteLogError(const char* info, const char* place, const char* other) const
{
    return m_cb.WriteLogError(info, place, other);
}
bool CSLWebRequest::WriteLogDebug(const char* info, const char* place, const char* other) const
{
    return m_cb.WriteLogDebug(info, place, other);
}
bool CSLWebRequest::WriteLogTrace(const char* info, const char* place, const char* other) const
{
    return m_cb.WriteLogTrace(info, place, other);
}
