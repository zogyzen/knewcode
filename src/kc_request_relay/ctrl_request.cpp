#include "ctrl_request.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlRelay类
CCtrlRelay::CCtrlRelay(IKCRelayREST& own, string sName, boost::property_tree::ptree& pt) : m_own(own), m_name(sName)
{
    if (pt.get_child_optional("<xmlattr>.server"))
        m_server = pt.get<string>("<xmlattr>.server");
}

CCtrlRelay::~CCtrlRelay()
{
}

// 回调函数
const char* CCtrlRelay::status_message(int status)
{
    return httplib::detail::status_message(status);
    // return httplib::status_message(status);
}
bool CCtrlRelay::WriteLogError(const char* info, const char* place, const char* other) const
{
    return m_own.WriteLogError(info, place, other);
}
bool CCtrlRelay::WriteLogDebug(const char* info, const char* place, const char* other) const
{
    return m_own.WriteLogDebug(info, place, other);
}
bool CCtrlRelay::WriteLogTrace(const char* info, const char* place, const char* other) const
{
    return m_own.WriteLogTrace(info, place, other);
}

// 响应头
void CCtrlRelay::RespondHeader(string sSrv, const KcHeaders& h)
{
    ParseCookie(sSrv, h, m_cookieMan);
}

// 解析cookie
void CCtrlRelay::ParseCookie(string sSrv, const httplib::Headers& h, CUtilHttp::TKCCookieManager& ckMan)
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
void CCtrlRelay::MakeCookie(string sSrv, string /*sPth*/, string &sClnCookies, httplib::Headers& h, CUtilHttp::TKCCookieManager& ckMan)
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

// 执行控制器
void CCtrlRelay::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sApiUrl;
    auto fExceptInfo = [&](void)
    {
        return (boost::format("\n (Relay-> [act=%s]) ") % pAct).str();
    };
    try
    {
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "post"));
        // 解析控制器
        map<string, string> mpCfgs;
        CCltrFunc::ParseCtrl(mpCfgs, CUtilFunc::PCharSafeToStr(pContext));
        // 调用前置的控制器
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
        if (0 != iErrCode)
            throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$Before] " + objCtrlD.JsonRespond().GetErrMsg(), m_own);
        // GET请求（method="get"）
        if ("get" == sMethod)
            sApiUrl = PostGet(objCtrlD, mpCfgs);
        // POST请求（method="post"）
        else
            sApiUrl = PostGet(objCtrlD, mpCfgs, true);
        string sErrMsg = objCtrlD.JsonRespond().GetErrMsg();
        // 如果失败，则退出
        if (0 != (iErrCode = objCtrlD.JsonRespond().GetErrCode()))
            //throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$request - " + sApiUrl + "]" + sErrMsg + "\n", m_own);
            throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$request]" + sErrMsg + "\n", m_own);
        // 调用后置的控制器
        if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true)))
            throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$after]" + sErrMsg, m_own);
    }
    catch (TKCRelayRESTException &ex)
    {
        string sErr = m_own.getHint("Request_Failed_") + sApiUrl + "\n" + ex.error_info();
        m_own.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, (act.GetSingleInfo("the_request") + string("\n") + pLocalFile).c_str());
        //throw;
        objCtrlD.SetJsonRespond(ex.error_id(), sErr.c_str());
    }
    catch (std::exception &ex)
    {
        string sErr = m_own.getHint("Request_Failed_") + sApiUrl + "\n" + ex.what();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, (act.GetSingleInfo("the_request") + fExceptInfo() + string("\n") + pLocalFile).c_str());
        //throw TKCRelayRESTException(ecd_ErrCode_KCRequestRelay + 3, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + ex.what() + fExceptInfo(), m_own);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCRequestRelay + 3, ex.what());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Request_Failed_") + sApiUrl + "\n" + m_own.getHint("Unknown_exception");
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, (act.GetSingleInfo("the_request") + string("\n") + pLocalFile).c_str());
        //throw TKCRelayRESTException(ecd_ErrCode_KCRequestRelay + 4, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Unknown_exception") + fExceptInfo(), m_own);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCRequestRelay + 4, m_own.getHint("Unknown_exception"));
    }
}

// 生成请求头
void CCtrlRelay::MakeRequestHeaders(KcHeaders &headers, ICtrlApiData &objCtrlD, map<string, string> &mpCfgs, string sSrv, string sPth)
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
            throw TKCRelayRESTException(ecd_ErrCode_KCRequestRelay + 5, __CURR_CODE_PLACE_C__, ec.message() + " \n" + sJsonHeaders, m_own);
        if (!jsnHeaders.is_object())
            throw TKCRelayRESTException(ecd_ErrCode_KCRequestRelay + 6, __CURR_CODE_PLACE_C__, "Json Format Error - Isn't a JsonObject \n" + sJsonHeaders, m_own);
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
    CCtrlRelay::MakeCookie(sSrv, sPth, sCookies, headers, m_cookieMan);
    std::cout << "\t 🍩 " << __FUNCTION__ << ": " << sCookies << std::endl;
    m_own.WriteLogTrace((boost::format(" 🍩 Request: %s \t%s") % sCookies % sJsonHeaders).str().c_str(), __CURR_CODE_PLACE_C__);
}

// 请求
string CCtrlRelay::PostGet(ICtrlApiData& objCtrlD, map<string, string> &mpCfgs, bool isPost)
{
    [[maybe_unused]] IActionData &act = objCtrlD.ActionData();
    // 服务器地址
    string sSrv = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$server");
    if (sSrv.empty())
        throw TKCRelayRESTException(ecd_ErrCode_KCRequestRelay + 7, __CURR_CODE_PLACE_C__, "$server Not Set", m_own);
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
