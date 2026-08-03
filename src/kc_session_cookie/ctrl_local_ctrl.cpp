#include "ctrl_local_ctrl.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlLocalCtrl
CCtrlLocalCtrl::CCtrlLocalCtrl(IKCSessionCookie& own, string sName, property_tree::ptree& pt)
    : ICtrlSession(own), m_name(sName)
{
    auto fGetUri = [&](string sName)
    {
        string sResult = "";
        if (pt.get_child_optional(sName))
        {
            auto ptItem = pt.get_child(sName);
            if (ptItem.get_child_optional("<xmlattr>.uri"))
                sResult = ptItem.get<string>("<xmlattr>.uri");
        }
        return sResult;
    };
    // 控制器
    m_ctrl_get = fGetUri("get");
    m_ctrl_set = fGetUri("set");
    m_ctrl_del = fGetUri("del");
    m_ctrl_nextval = fGetUri("nextval");
}

// 前缀名
string CCtrlLocalCtrl::PrefixName(string sSessionID, string sClientID) const
{
    return c_RESTful_KCSession + m_name + "_" + sSessionID + "_" + sClientID + "__";
}
// 添加Session（sNameFull = PrefixName(参数) + 短名）
void CCtrlLocalCtrl::SetSession(string sNameFull, string sVal, IActionData* pAct)
{
    auto ssPair = GetCookieParm(sNameFull, sVal);
    string sRespond;
    if (nullptr == pAct)
        sRespond = CCltrFunc::CallCtrl(m_own, m_ctrl_set, ssPair.first, ssPair.second);
    else
        sRespond = CCltrFunc::CallCtrl(*pAct, m_own, m_ctrl_set, ssPair.first, ssPair.second);
    system::error_code ec;
    auto jsnResult = boost::json::parse(sRespond, ec);
    if (ec.failed()) throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 3, __CURR_CODE_PLACE_C__, ec.message() + " - " + sNameFull + " \n" + sRespond, m_own);
    int iErrCode = -1;
    try
    {
        iErrCode = static_cast<int>(jsnResult.at(c_RESTful_errCode).as_int64());
        if (0 != iErrCode)
            throw TKCSessionCookieException(iErrCode, __CURR_CODE_PLACE_C__, GetJsonValue(jsnResult.at(c_RESTful_errMsg)) + "\n  " + m_ctrl_set + " - " + sNameFull, m_own);
    }
    catch (std::exception &ex)
    {
        throw TKCSessionCookieException(iErrCode, __CURR_CODE_PLACE_C__, ex.what() + string("\n  ") + m_ctrl_set + " - " + sNameFull,
                                        m_own, string("<") + typeid(ex).name() + "> " + sRespond);
    }
}
// 得到Session
string CCtrlLocalCtrl::GetSession(string sNameFull, IActionData* pAct)
{
    return GetSessionVal(sNameFull, m_ctrl_get, pAct);
}
// 得到下一个自增值
string CCtrlLocalCtrl::GetSessionNextVal(string sNameFull, string /*sValBegin*/, IActionData* pAct)
{
    return GetSessionVal(sNameFull, m_ctrl_nextval, pAct);
}
// 获取session值
string CCtrlLocalCtrl::GetSessionVal(string sNameFull, string ctrlUrl, IActionData* pAct)
{
    auto ssPair = GetCookieParm(sNameFull);
    string sRespond;
    if (nullptr == pAct)
        sRespond = CCltrFunc::CallCtrl(m_own, ctrlUrl, ssPair.first, ssPair.second);
    else
        sRespond = CCltrFunc::CallCtrl(*pAct, m_own, ctrlUrl, ssPair.first, ssPair.second);
    system::error_code ec;
    auto jsnResult = json::parse(sRespond, ec);
    if (ec.failed())
        throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 4, __CURR_CODE_PLACE_C__, ec.message() + " - " + sNameFull + " \n" + sRespond, m_own);
    int iErrCode = -1;
    try
    {
        iErrCode = static_cast<int>(jsnResult.at(c_RESTful_errCode).as_int64());
        if (0 != iErrCode)
            throw TKCSessionCookieException(iErrCode, __CURR_CODE_PLACE_C__, GetJsonValue(jsnResult.at(c_RESTful_errMsg)) + "\n  " + ctrlUrl + " - " + sNameFull, m_own);
    }
    catch (std::exception &ex)
    {
        throw TKCSessionCookieException(iErrCode, __CURR_CODE_PLACE_C__, ex.what() + string("\n  ") + ctrlUrl + " - " + sNameFull,
                                        m_own, string("<") + typeid(ex).name() + "> \n" + sRespond);
    }
    // auto val = jsnResult.try_at(c_RESTful_val);
    // if (!val.has_value())
    //     throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 5, __CURR_CODE_PLACE_C__, string() + m_own.getHint("Don_t_exists_field_") + c_RESTful_val + ". " + ctrlUrl + " \n  " + sNameFull, m_own);
    // return GetJsonValue(val.value());
    return GetJsonValue(jsnResult.at(c_RESTful_val));
}
// 删除Session
void CCtrlLocalCtrl::DelSession(string sNameFull, IActionData* pAct)
{
    auto ssPair = GetCookieParm(sNameFull);
    string sRespond;
    if (nullptr == pAct)
        sRespond = CCltrFunc::CallCtrl(m_own, m_ctrl_del, ssPair.first, ssPair.second);
    else
        sRespond = CCltrFunc::CallCtrl(*pAct, m_own, m_ctrl_del, ssPair.first, ssPair.second);
    system::error_code ec;
    auto jsnResult = json::parse(sRespond, ec);
    if (ec.failed()) throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 5, __CURR_CODE_PLACE_C__, ec.message() + " - " + sNameFull + " \n" + sRespond, m_own);
    int iErrCode = -1;
    try
    {
        iErrCode = static_cast<int>(jsnResult.at(c_RESTful_errCode).as_int64());
        if (0 != iErrCode)
            throw TKCSessionCookieException(iErrCode, __CURR_CODE_PLACE_C__, GetJsonValue(jsnResult.at(c_RESTful_errMsg)) + "\n  " + m_ctrl_del + " - " + sNameFull, m_own);
    }
    catch (std::exception &ex)
    {
        throw TKCSessionCookieException(iErrCode, __CURR_CODE_PLACE_C__, ex.what() + string("\n  ") + m_ctrl_del + " - " + sNameFull,
                                        m_own, string("<") + typeid(ex).name() + "> \n" + sRespond);
    }
}
// 获取json值
string CCtrlLocalCtrl::GetJsonValue(json::value val)
{
    string sVal;
    try
    {
        if (val.is_null()) ;
        else if (val.is_double()) sVal = std::to_string(val.as_double());
        else if (val.is_int64()) sVal = std::to_string(val.as_int64());
        else if (val.is_array() || val.is_object()) sVal = json::serialize(val);
        else sVal = val.as_string().c_str();
    }
    catch (std::exception &ex)
    {
        throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 5, __CURR_CODE_PLACE_C__, string("<") + typeid(ex).name() + "> " + ex.what(), m_own);
    }
    return sVal;
}

// 设置期限（d：天。h：小时。s：秒。）
void CCtrlLocalCtrl::SetExpire(string /*sNameFull*/, string /*exp*/, IActionData* /*pAct*/)
{
}

// 分解名称
void CCtrlLocalCtrl::SplitName(string sNameFull, string& sSessionID, string& sClientID, string& sName)
{
    // 分解（todo: 客户端标识放在后面）
    string sSplit = sNameFull.substr((c_RESTful_KCSession + m_name + "_").size());
    int iSplit = static_cast<int>(sSplit.find("__"));
    // 名称
    sName = sSplit.substr(iSplit + 2);
    // 客户端标识
    string sClnFlag = sSplit.substr(0, iSplit);
    // 分解客户端标识
    vector<string> idsVec;
    algorithm::split(idsVec, sClnFlag, is_any_of("_"));
    if (idsVec.size() < 2)
        throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 6, __CURR_CODE_PLACE_C__, string(m_own.getHint("Parameter_type_unknown_")) + sNameFull, m_own);
    sSessionID = idsVec[0];
    sClientID = idsVec[1];
}

// 获取cookie和参数
pair<string, string> CCtrlLocalCtrl::GetCookieParm(string sNameFull, string sVal)
{
    string sSessionID, sClientID, sName;
    SplitName(sNameFull, sSessionID, sClientID, sName);
    string sCookie = "KCCLNID=" + sClientID + "; KCSSID=" + sSessionID;
    string sParm = (format(R"({"ctrl":"%s","name":"%s","fname":"%s","%s":"%s"})") % m_name % sName % (c_RESTful_KCSession + m_name + "__" + sName) % c_RESTful_val % sVal).str();
    return pair<string, string>(sCookie, sParm);
}

// 转换时间
posix_time::ptime CCtrlLocalCtrl::MakePTime(string sExpire)
{
    char unt = sExpire.back();
    int iVal = lexical_cast<int>(sExpire.substr(0, sExpire.size() - 1));
    if ('d' == unt)
        return posix_time::second_clock::local_time() + gregorian::days(iVal);
    else if ('h' == unt)
        return posix_time::second_clock::local_time() + posix_time::hours(iVal);
    else if ('s' == unt)
        return posix_time::second_clock::local_time() + posix_time::seconds(iVal);
    else
        throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 7, __CURR_CODE_PLACE_C__, string(m_own.getHint("Parameter_type_unknown_")) + sExpire, m_own);
}

void CCtrlLocalCtrl::GC(void)
{
}
