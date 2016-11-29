#include "../std.h"
#include "var_bool.h"

////////////////////////////////////////////////////////////////////////////////
// TVarBool类
KC::one_level::TVarBool::TVarBool(IIDNameItemWork& wk, IKCActionData& act, string name)
    : TVarItemBase(wk, act, EKcDtBool, name)
{
}

// 变量赋值
void KC::one_level::TVarBool::SetVarValue(boost::any val)
{
    if (val.empty())
        m_value = false;
    else if (val.type() == typeid(bool))
        m_value = boost::any_cast<bool>(val);
    else if (val.type() == typeid(int))
        m_value = boost::any_cast<int>(val) > 0;
    else if (val.type() == typeid(double))
        m_value = boost::any_cast<double>(val) > 0;
    else if (val.type() == typeid(string))
        m_value = CUtilFunc::StrToLower(boost::any_cast<string>(val)) == "true";
    else
        throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_work.getHint("Type_unmatch")) % m_varName % m_act.GetCurrLineID()).str(), m_work, m_varName);
}

// 获取变量的值
boost::any KC::one_level::TVarBool::GetVarValue(void)
{
    return m_value;
}
