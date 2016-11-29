#include "../std.h"
#include "invar_get.h"

////////////////////////////////////////////////////////////////////////////////
// TInvarGet类
KC::one_level::TInvarGet::TInvarGet(IKCActionData& act, CInnerVar& iv) : m_act(act), m_InnerVar(iv)
{
    const char* pGet = m_act.GetRequestRespond().GetGetArgStr();
    if (nullptr != pGet)
    {
        string sGet = pGet;
        TInvarGet::InitParms(m_mapArg, sGet);
    }
}

// 得到名称
const char* KC::one_level::TInvarGet::GetName(void)
{
    return TInvarGet::GetNameS();
}
const char* KC::one_level::TInvarGet::GetNameS(void)
{
    static string sName = string("Innervar-") + g_InnerVarGet;
    return sName.c_str();
}

// 获取下标
string KC::one_level::TInvarGet::GetArrIndex(boost::any arr)
{
    string sArr = "";
    int iArr = -1;
    if (arr.type() == typeid(int)) iArr = boost::any_cast<int>(arr);
    else if (arr.type() == typeid(bool)) iArr = boost::any_cast<bool>(arr) ? 1 : 0;
    else if (arr.type() == typeid(double)) iArr = (int)boost::any_cast<double>(arr);
    // 整型参数
    if (iArr > -1)
        sArr = lexical_cast<string>(iArr);
    // 字符串参数
    else
        sArr = boost::any_cast<string>(arr);
    return sArr;
}

// 得到内部变量值
bool KC::one_level::TInvarGet::Exists(boost::any arr)
{
    auto it = m_mapArg.find(TInvarGet::GetArrIndex(arr));
    return m_mapArg.end() != it;
}

// 得到内部变量值
boost::any KC::one_level::TInvarGet::GetValue(boost::any arr)
{
    return TInvarGet::GetStringValue(m_mapArg, arr);
}

// 设置内部变量值
void KC::one_level::TInvarGet::SetValue(boost::any arr, boost::any)
{
    // throw TInnerVarException(m_act.GetCurrLineID(), __FUNCTION__, "Can't set the value, by inner variable get[\"" + TInvarGet::GetArrIndex(arr) + "\"], at line " + lexical_cast<string>(m_act.GetCurrLineID()), m_InnerVar);
}
