#include "../std.h"
#include "invar_cookie.h"

////////////////////////////////////////////////////////////////////////////////
// TInvarGet类
KC::two_level::TInvarCookie::TInvarCookie(IKCActionData& act, CInnerVar& iv) : m_act(act), m_re(act.GetRequestRespond()), m_InnerVar(iv)
{
    const char* pCookie = m_re.GetCookieStr();
    if (nullptr != pCookie)
    {
        string sCookie = pCookie;
        one_level::TInvarGet::InitParms(m_CookieList, sCookie, ";");
    }
}

KC::two_level::TInvarCookie::~TInvarCookie()
{
    for (auto it = m_CookieList.begin(); m_CookieList.end() != it; ++it)
        if (it->second.new_send && it->first.length() > 0)
            m_re.AddCookie(it->first.c_str(), it->second.val.c_str(), this->TimeToStr(it->second.expires).c_str(), it->second.path.c_str(), it->second.domain.c_str());
}

// 得到名称
const char* KC::two_level::TInvarCookie::GetName(void)
{
    return TInvarCookie::GetNameS();
}
const char* KC::two_level::TInvarCookie::GetNameS(void)
{
    static string sName = string("Innervar-") + g_InnerCookie;
    return sName.c_str();
}

// 转换时间
inline string KC::two_level::TInvarCookie::TimeToStr(posix_time::ptime tm) const
{
    return tm.is_not_a_date_time() ? "" : boost::replace_all_copy(posix_time::to_iso_extended_string(tm), "T", " ");
}
inline posix_time::ptime KC::two_level::TInvarCookie::StrToTime(string str) const
{
    try
    {
        return posix_time::time_from_string(str);
    }
    catch(...)
    {
        throw TInnerVarException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_InnerVar.getHint("Datetime_Format_Error")) % str % m_act.GetCurrLineID()).str(), m_InnerVar);
    }
}

// 得到内部变量值
bool KC::two_level::TInvarCookie::Exists(TAnyTypeValList& arrs)
{
    auto it = m_CookieList.find(one_level::TInvarGet::GetArrIndex(arrs.front()));
    return m_CookieList.end() != it;
}

// 得到cookie值
boost::any KC::two_level::TInvarCookie::GetValue(TAnyTypeValList& arrs)
{
    return one_level::TInvarGet::GetStringValue(m_CookieList, arrs.front());
}

// 设置cookie值
void KC::two_level::TInvarCookie::SetValue(TAnyTypeValList& arrs, boost::any val)
{
    string sName = one_level::TInvarGet::GetArrIndex(arrs.front());
    m_CookieList[sName].name = sName;
    m_CookieList[sName].new_send = true;
    // 值
    string sVal;
    if (val.type() == typeid(double))
        sVal = lexical_cast<string>(boost::any_cast<double>(val));
    else if (val.type() == typeid(int))
        sVal = lexical_cast<string>(boost::any_cast<int>(val));
    else if (val.type() == typeid(bool))
        sVal = boost::any_cast<bool>(val) ? "true" : "false";
    else
        sVal = boost::any_cast<string>(val);
    // 设置
    if (arrs.size() > 1)
    {
        string sNameEx = one_level::TInvarGet::GetArrIndex(arrs[1]);
        if ("path" == sNameEx)
            m_CookieList[sName].path = sVal;
        else if ("domain" == sNameEx)
            m_CookieList[sName].domain = sVal;
        else if ("expires" == sNameEx)
            m_CookieList[sName].expires = this->StrToTime(sVal);
    }
    else
        m_CookieList[sName].val = sVal;
}
