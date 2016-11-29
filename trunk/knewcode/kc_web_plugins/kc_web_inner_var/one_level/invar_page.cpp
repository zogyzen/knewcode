#include "../std.h"
#include "invar_page.h"

////////////////////////////////////////////////////////////////////////////////
// TInvarPage类
KC::one_level::TInvarPage::TInvarPage(IKCActionData& act, CInnerVar& iv) : m_act(act), m_InnerVar(iv)
{
}

// 得到名称
const char* KC::one_level::TInvarPage::GetName(void)
{
    return TInvarPage::GetNameS();
}
const char* KC::one_level::TInvarPage::GetNameS(void)
{
    static string sName = string("Innervar-") + g_InnerVarPage;
    return sName.c_str();
}

// 得到内部变量值
bool KC::one_level::TInvarPage::Exists(boost::any arr)
{
    string sArr = CUtilFunc::StrToLower(boost::any_cast<string>(arr));
    if ("all_info" == sArr)
        return true;
    else if ("all_info_html" == sArr)
        return true;
    else if ("website_local_path" == sArr)
        return true;
    else if ("page_local_path" == sArr)
        return true;
    else if ("page_local_file" == sArr)
        return true;
    else if ("page_uri_file" == sArr)
        return true;
    else if ("Respond-Content-Type" == sArr)
        return true;
    else if ("this" == sArr)
        return true;
    else
        return false;
}

// 得到内部变量值
boost::any KC::one_level::TInvarPage::GetValue(IActionData& act, boost::any arr)
{
    if (arr.type() == typeid(string))
    {
        string sArr = CUtilFunc::StrToLower(boost::any_cast<string>(arr));
        if ("all_info" == sArr)
            return string(m_act.GetRequestRespond().GetAllInfo("\n"));
        else if ("all_info_html" == sArr)
            return string(m_act.GetRequestRespond().GetAllInfo("<br>"));
        else if ("website_local_path" == sArr)
            return string(m_act.GetRequestRespond().GetLocalRootPath());
        else if ("page_local_path" == sArr)
            return string(m_act.GetRequestRespond().GetLocalPagePath());
        else if ("page_local_file" == sArr)
            return string(m_act.GetRequestRespond().GetLocalFilename());
        else if ("page_uri_file" == sArr)
            return string(m_act.GetRequestRespond().GetUriFilename());
        else if ("Respond-Content-Type" == sArr)
            return string(m_act.GetRequestRespond().GetResponseContentType());
        else if ("this" == sArr)
            return TKcWebInfVal(sArr, (TObjectPointer)(&act));
    }
    return string("");
}

// 设置内部变量值
void KC::one_level::TInvarPage::SetValue(boost::any arr, boost::any val)
{
    if (arr.type() == typeid(string))
    {
        string sArr = CUtilFunc::StrToLower(boost::any_cast<string>(arr));
        if ("Respond-Content-Type" == sArr)
        {
            if (val.type() == typeid(string))
                m_act.GetRequestRespond().SetResponseContentType(boost::any_cast<string>(val).c_str());
        }
    }
}
