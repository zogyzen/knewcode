#include "../std.h"
#include "var_string.h"

////////////////////////////////////////////////////////////////////////////////
// TVarString类
KC::one_level::TVarString::TVarString(IIDNameItemWork& wk, IKCActionData& act, string name)
    : TVarItemBase(wk, act, EKcDtStr, name)
{
}

// 变量赋值
void KC::one_level::TVarString::SetVarValue(boost::any val)
{
    if (val.empty())
        m_value = "";
    else if (val.type() == typeid(double))
        m_value = lexical_cast<string>(boost::any_cast<double>(val));
    else if (val.type() == typeid(int))
        m_value = lexical_cast<string>(boost::any_cast<int>(val));
    else if (val.type() == typeid(bool))
        m_value = boost::any_cast<bool>(val) ? "true" : "false";
    else if (val.type() == typeid(string))
        m_value = boost::any_cast<string>(val);
    else
        throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_work.getHint("Type_unmatch")) % m_varName % m_act.GetCurrLineID()).str(), m_work, m_varName);
}

// 获取变量的值
boost::any KC::one_level::TVarString::GetVarValue(void)
{
    return m_value;
}
