#include "../std.h"
#include "invar_path.h"

////////////////////////////////////////////////////////////////////////////////
// TInvarPath类
KC::one_level::TInvarPath::TInvarPath(IKCActionData& act, CInnerVar& iv) : m_act(act), m_InnerVar(iv)
{
}

// 得到名称
const char* KC::one_level::TInvarPath::GetName(void)
{
    return TInvarPath::GetNameS();
}
const char* KC::one_level::TInvarPath::GetNameS(void)
{
    static string sName = string("Innervar-") + g_InnerVarPath;
    return sName.c_str();
}

// 得到内部变量值
bool KC::one_level::TInvarPath::Exists(boost::any arr)
{
    return true;
}

// 得到内部变量值
boost::any KC::one_level::TInvarPath::GetValue(boost::any arr)
{
    if (arr.type() == typeid(string))
    {
        string sArr = boost::algorithm::trim_copy(boost::any_cast<string>(arr));
        return m_act.GetFullPath(sArr);
    }
    return string("");
}

// 设置内部变量值
void KC::one_level::TInvarPath::SetValue(boost::any arr, boost::any val)
{
}
