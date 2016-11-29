#include "../std.h"
#include "invar_user.h"

////////////////////////////////////////////////////////////////////////////////
// TInnerVarKey类
KC::one_level::TInvarUser::TInnerVarKey::TInnerVarKey(boost::any k) : key(k)
{
}
KC::one_level::TInvarUser::TInnerVarKey::TInnerVarKey(const KC::one_level::TInvarUser::TInnerVarKey& clone) : key(clone.key)
{
}

// 拷贝
KC::one_level::TInvarUser::TInnerVarKey KC::one_level::TInvarUser::TInnerVarKey::operator=(const KC::one_level::TInvarUser::TInnerVarKey& rv)
{
    key = rv.key;
    return *this;
}

// 比较
bool KC::one_level::TInvarUser::TInnerVarKey::operator<(const KC::one_level::TInvarUser::TInnerVarKey& rv) const
{
    bool bResult = false;
    if (key.type() == rv.key.type())
    {
        if (key.type() == typeid(bool))
            bResult = boost::any_cast<bool>(key) < boost::any_cast<bool>(rv.key);
        else if (key.type() == typeid(int))
            bResult = boost::any_cast<int>(key) < boost::any_cast<int>(rv.key);
        else if (key.type() == typeid(double))
            bResult = boost::any_cast<double>(key) < boost::any_cast<double>(rv.key);
        else if (key.type() == typeid(string))
            bResult = boost::any_cast<string>(key) < boost::any_cast<string>(rv.key);
        else if (key.type() == typeid(TKcWebInfVal))
            bResult = boost::any_cast<TKcWebInfVal>(key).pointer < boost::any_cast<TKcWebInfVal>(rv.key).pointer;
    }
    else
    {
        if  (
            (key.type() == typeid(bool))
            || (key.type() == typeid(int) && rv.key.type() != typeid(bool))
            || (key.type() == typeid(double) && rv.key.type() != typeid(bool) && rv.key.type() != typeid(int))
            || (key.type() == typeid(string) && rv.key.type() != typeid(bool) && rv.key.type() != typeid(int) && rv.key.type() != typeid(double))
            )
            bResult = true;
        else
            bResult = false;
    }
    return bResult;
}

////////////////////////////////////////////////////////////////////////////////
// TInvarUser类
KC::one_level::TInvarUser::TInvarUser(IKCActionData& act, CInnerVar& iv) : m_act(act), m_InnerVar(iv)
{
}

// 得到名称
const char* KC::one_level::TInvarUser::GetName(void)
{
    return TInvarUser::GetNameS();
}
const char* KC::one_level::TInvarUser::GetNameS(void)
{
    static string sName = string("Innervar-") + g_InnerVarUser;
    return sName.c_str();
}

// 得到内部变量值
bool KC::one_level::TInvarUser::Exists(boost::any arr)
{
    TInnerVarKey arrKey(arr);
    auto iter = m_UserInVarList.find(arrKey);
    return m_UserInVarList.end() != iter;
}

// 得到内部变量值
boost::any KC::one_level::TInvarUser::GetValue(boost::any arr)
{
    return this->GetUserInVar(arr);
}

// 设置内部变量值
void KC::one_level::TInvarUser::SetValue(boost::any arr, boost::any val)
{
    this->SetUserInVar(arr, val);
}

// 添加user内部变量
void KC::one_level::TInvarUser::SetUserInVar(boost::any arr, boost::any val)
{
    TInnerVarKey arrKey(arr);
    auto iter = m_UserInVarList.find(arrKey);
    if (m_UserInVarList.end() == iter)
        m_UserInVarList.insert(make_pair(arrKey, val));
    else
        m_UserInVarList[arrKey] = val;
}

// 获取user内部变量
boost::any KC::one_level::TInvarUser::GetUserInVar(boost::any arr)
{
    auto iter = m_UserInVarList.find(TInnerVarKey(arr));
    if (m_UserInVarList.end() == iter)
        return string("");
    else
        return iter->second;
}
