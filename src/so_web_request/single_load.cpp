#include "single_load.h"

// 引出函数
extern "C"
{
    // 创建实例
    ISLWebRequest& create(ISLWebRequestCB& cb)
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
CSLWebRequest::CSLWebRequest(ISLWebRequestCB& cb) : m_cb(cb)
{
}

CSLWebRequest::~CSLWebRequest(void)
{
}

// 解析cookie
void CSLWebRequest::ParseCookie(string sSrv, const httplib::Headers& h, CUtilHttp::TKCCookieManager& ckMan)
{
    string sSetCkFlag = "Set-Cookie";
    if (h.count(sSetCkFlag) > 0)
    {
        auto ckLst = ckMan.getOrCreateCookieList(sSrv);
        // 服务器端设置的cookie列表
        auto itCks = h.equal_range(sSetCkFlag);
        for (auto it = itCks.first; itCks.second != it; ++it)
        {
            //cout << it->second << endl;
            ckLst->setCookie(it->second);
        }
    }
}
// 生成cookie
void CSLWebRequest::MakeCookie(string sSrv, string /*sPth*/, string &sClnCookies, httplib::Headers& h, CUtilHttp::TKCCookieManager& ckMan)
{
    auto ckLst = ckMan.getCookieList(sSrv);
    string sCks = ckLst.get() != nullptr ? ckLst->makeCookie(sClnCookies) : sClnCookies;
    if (!sCks.empty())
    {
        h.erase(c_RequestHeaderCookie);
        h.insert(std::make_pair(c_RequestHeaderCookie, sCks));
    }
    sClnCookies = sCks;
}

// 转发请求
string CSLWebRequest::PostGet(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs, bool isPost)
{
    [[maybe_unused]] IActionData &act = objCtrlD.ActionData();
    // 服务器地址
    string sSrv = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$server");
    if (sSrv.empty())
        throw std::runtime_error((boost::format("%s\n ❌ `$server` Not Set - %s") % __CURR_CODE_PLACE_C__ % objCtrlD.Url()).str());
    // 路径
    string sPth = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$path");
    // 参数
    string sPrm = CCltrFunc::ReplaceParm(objCtrlD, mpCfgs, "$parm");
    if (!sPrm.empty()) sPrm = "?" + sPrm;
    // 请求body
    string sRequstBody;
    if (isPost)
    {
        // 数据部分
        sRequstBody = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$data");
        // 为空，则取上一个控制器的返回
        if (sRequstBody.empty() && objCtrlD.JsonRespondLast().GetItemCount() > 0)
            sRequstBody = objCtrlD.JsonRespondLast().GetStr();
        // 为空，则取子控制器的最终结果
        if (sRequstBody.empty() && objCtrlD.JsonRespond().GetItemCount() > 0)
            sRequstBody = objCtrlD.JsonRespond().GetStr();
        // 仍为空，则取输入参数
        if (sRequstBody.empty())
            sRequstBody = objCtrlD.JsonRequest().GetStr();
    }
    // 请求头
    KcHeaders headers;
    MakeRequestHeaders(headers, objCtrlD, mpCfgs, sSrv, sPth);
    // 超时设置
    unsigned iTimeout = static_cast<unsigned>(objCtrlD.JsonAttach().GetVal("timeout", 30));
    // 延迟设置
    int iDelay = static_cast<int>(objCtrlD.JsonAttach().GetVal("delay", 0));
    if (iDelay > 0)
        boost::this_thread::sleep(boost::posix_time::milliseconds(iDelay));
    // 请求
    string sRespond = _PostGet(*this, headers, sSrv, sPth, sPrm, sRequstBody, isPost, iTimeout);
    // 重置应答json
    objCtrlD.JsonRespond().ResetByStr(sRespond.c_str());
    return sSrv + sPth + sPrm;
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
        MakeCookie(sSrv, sPth, sClnCookies, headers, m_cookieMan);
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
        MakeCookie(sSrv, sPth, sClnCookies, headers, m_cookieMan);
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

// 生成请求头
void CSLWebRequest::MakeRequestHeaders(KcHeaders &headers, ICtrlApiData &objCtrlD, map<string, string> &mpCfgs, string sSrv, string sPth)
{
    IActionData &act = objCtrlD.ActionData();
    // 客户端cookie值
    string sCookies = CUtilFunc::PCharSafeToStr(act.GetSingleInfo(c_RequestHeaderCookie));
    // 请求头
    string sJsonHeaders = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$headers");
    if (!sJsonHeaders.empty())
    {
        // 解析json
        boost::system::error_code ec;
        auto jsnHeaders = boost::json::parse(sJsonHeaders, ec);
        if (ec.failed())
            throw std::runtime_error((boost::format("%s\n ❌ %s\n%s\n%s") % __CURR_CODE_PLACE_C__ % ec.message() % objCtrlD.Url() % sJsonHeaders).str());
        if (!jsnHeaders.is_object())
            throw std::runtime_error((boost::format("%s\n%s\n ❌ Json Format Error - Isn't a JsonObject\n%s") % __CURR_CODE_PLACE_C__ % objCtrlD.Url() % sJsonHeaders).str());
        // 循环每个头信息
        for (auto &jsn: jsnHeaders.as_object())
        {
            string sKey = jsn.key();
            string sVal = jsn.value().as_string().c_str();
            // sVal = CCltrFunc::GetParmVal(objCtrlD, sVal, sJsonHeaders);
            if (!sVal.empty())
            {
                // cookie另处理
                if (c_RequestHeaderCookie == sKey)
                    sCookies = sVal;
                // 添加请求头
                else
                    headers.insert(std::make_pair(sKey, sVal));
            }
        }
    }
    // 添加cookie
    CSLWebRequest::MakeCookie(sSrv, sPth, sCookies, headers, m_cookieMan);
    std::cout << "\t 🍩 " << __FUNCTION__ << ": " << sCookies << std::endl;
    WriteLogTrace((boost::format(" 🍩 Request: %s \t%s") % sCookies % sJsonHeaders).str().c_str(), __CURR_CODE_PLACE_C__);
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
    ParseCookie(sSrv, h, m_cookieMan);
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
