#include "../std.h"
#include "var_int.h"

////////////////////////////////////////////////////////////////////////////////
// TVarInt类
KC::one_level::TVarInt::TVarInt(IIDNameItemWork& wk, IKCActionData& act, string name)
    : TVarItemBase(wk, act, EKcDtInt, name)
{
}

// 变量赋值
void KC::one_level::TVarInt::SetVarValue(boost::any val)
{
    if (val.empty())
        m_value = 0;
    else if (val.type() == typeid(int))
        m_value = boost::any_cast<int>(val);
    else if (val.type() == typeid(double))
        m_value = (int)boost::any_cast<double>(val);
    else if (val.type() == typeid(bool))
        m_value = boost::any_cast<bool>(val) ? 1 : 0;
    else if (val.type() == typeid(string))
    {
        string strVal = boost::any_cast<string>(val);
        if (strVal.empty()) m_value = 0;
        else
            try
            {
                m_value = lexical_cast<int>(strVal);
            }
            catch(...)
            {
                throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_work.getHint("Type_mismatch")) % m_varName % strVal % m_act.GetCurrLineID()).str(), m_work, m_varName);
            }
    }
    else
        throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_work.getHint("Type_unmatch")) % m_varName % m_act.GetCurrLineID()).str(), m_work, m_varName);
}

// 获取变量的值
boost::any KC::one_level::TVarInt::GetVarValue(void)
{
    return m_value;
}
