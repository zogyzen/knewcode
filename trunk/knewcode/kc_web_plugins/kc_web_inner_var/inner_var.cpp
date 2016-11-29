#include "std.h"
#include "inner_var.h"

#include "one_level/invar_get.h"
#include "one_level/invar_page.h"
#include "one_level/invar_path.h"
#include "one_level/invar_post.h"
#include "one_level/invar_user.h"
using namespace one_level;

#include "two_level/invar_cookie.h"
using namespace two_level;

// 得到内部变量数据
template<typename TInVar>
TInVar& GetInVarObj(IKCActionData& act, CInnerVar& man)
{
    IKCRequestRespond::IReStepData *pInVar = nullptr;
    // 如果在页请求过程中的数据不存在，则添加
    if (!act.GetRequestRespond().ExistsReStepData(TInVar::GetNameS()))
    {
        pInVar = new TInVar(dynamic_cast<IKCActionData&>(act.GetActRoot()), man);
        if (!act.GetRequestRespond().AddReStepData(TInVar::GetNameS(), pInVar))
            throw TInnerVarException(act.GetCurrLineID(), __FUNCTION__, (boost::format(man.getHint("Add_inner_variable_failure")) % TInVar::GetNameS() % act.GetCurrLineID()).str(), man);
    }
    // 如果在页请求过程中的数据存在，则读取
    else
    {
        pInVar = &act.GetRequestRespond().GetReStepData(TInVar::GetNameS());
        if (nullptr == dynamic_cast<TInVar*>(pInVar))
            throw TInnerVarException(act.GetCurrLineID(), __FUNCTION__, (boost::format(man.getHint("Isn_t_a_inner_variables")) % TInVar::GetNameS() % act.GetCurrLineID()).str(), man);
    }
    return dynamic_cast<TInVar&>(*pInVar);
}

////////////////////////////////////////////////////////////////////////////////
// CInnerVar类
CInnerVar::CInnerVar(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle)
{
}

CInnerVar::~CInnerVar()
{
}

// 得到服务特征码
const char* CInnerVar::getGUID(void) const
{
    return c_InnerVarSrvGUID;
}

// 对应的模块
const IBundle& CInnerVar::getBundle(void) const
{
    return m_bundle;
}

// 是否存在
bool CInnerVar::Exists(IActionData& act, string name, TAnyTypeValList& arr) const
{
    IKCActionData& actKc = dynamic_cast<IKCActionData&>(act);
    if (arr.empty())
        throw TInnerVarException(actKc.GetCurrLineID(), __FUNCTION__, (boost::format(this->getHint("The_array_index_of_inner_variable_is_empty")) % name % actKc.GetCurrLineID()).str(), const_cast<CInnerVar&>(*this));
    if (g_InnerVarGet == name)
        return GetInVarObj<TInvarGet>(actKc, const_cast<CInnerVar&>(*this)).Exists(arr.front());
    else if (g_InnerVarPost == name)
        return GetInVarObj<TInvarPost>(actKc, const_cast<CInnerVar&>(*this)).Exists(arr.front());
    else if (g_InnerVarPage == name)
        return GetInVarObj<TInvarPage>(actKc, const_cast<CInnerVar&>(*this)).Exists(arr.front());
    else if (g_InnerVarPath == name)
        return GetInVarObj<TInvarPath>(actKc, const_cast<CInnerVar&>(*this)).Exists(arr.front());
    else if (g_InnerVarUser == name)
        return GetInVarObj<TInvarUser>(actKc, const_cast<CInnerVar&>(*this)).Exists(arr.front());
    else if (g_InnerCookie == name)
        return GetInVarObj<TInvarCookie>(actKc, const_cast<CInnerVar&>(*this)).Exists(arr);
    else
        throw TInnerVarException(actKc.GetCurrLineID(), __FUNCTION__, (boost::format(this->getHint("Don_t_exists_inner_variable")) % name % actKc.GetCurrLineID()).str(), const_cast<CInnerVar&>(*this));
}

// 得到内部变量值
boost::any CInnerVar::GetValue(IActionData& act, string name, TAnyTypeValList& arr)
{
    IKCActionData& actKc = dynamic_cast<IKCActionData&>(act);
    if (arr.empty())
        throw TInnerVarException(actKc.GetCurrLineID(), __FUNCTION__, (boost::format(this->getHint("The_array_index_of_inner_variable_is_empty")) % name % actKc.GetCurrLineID()).str(), *this);
    if (g_InnerVarGet == name)
        return GetInVarObj<TInvarGet>(actKc, *this).GetValue(arr.front());
    else if (g_InnerVarPost == name)
        return GetInVarObj<TInvarPost>(actKc, *this).GetValue(arr.front());
    else if (g_InnerVarPage == name)
        return GetInVarObj<TInvarPage>(actKc, *this).GetValue(act, arr.front());
    else if (g_InnerVarPath == name)
        return GetInVarObj<TInvarPath>(actKc, *this).GetValue(arr.front());
    else if (g_InnerVarUser == name)
        return GetInVarObj<TInvarUser>(actKc, *this).GetValue(arr.front());
    else if (g_InnerCookie == name)
        return GetInVarObj<TInvarCookie>(actKc, *this).GetValue(arr);
    else
        throw TInnerVarException(actKc.GetCurrLineID(), __FUNCTION__, (boost::format(this->getHint("Don_t_exists_inner_variable")) % name % actKc.GetCurrLineID()).str(), *this);
}

// 设置内部变量值
void CInnerVar::SetValue(IActionData& act, string name, TAnyTypeValList& arr, boost::any val)
{
    IKCActionData& actKc = dynamic_cast<IKCActionData&>(act);
    if (arr.empty())
        throw TInnerVarException(actKc.GetCurrLineID(), __FUNCTION__, (boost::format(this->getHint("The_array_index_of_inner_variable_is_empty")) % name % actKc.GetCurrLineID()).str(), *this);
    if (g_InnerVarGet == name)
        GetInVarObj<TInvarGet>(actKc, *this).SetValue(arr.front(), val);
    else if (g_InnerVarPost == name)
        GetInVarObj<TInvarPost>(actKc, *this).SetValue(arr.front(), val);
    else if (g_InnerVarPage == name)
        GetInVarObj<TInvarPage>(actKc, *this).SetValue(arr.front(), val);
    else if (g_InnerVarPath == name)
        GetInVarObj<TInvarPath>(actKc, *this).SetValue(arr.front(), val);
    else if (g_InnerVarUser == name)
        GetInVarObj<TInvarUser>(actKc, *this).SetValue(arr.front(), val);
    else if (g_InnerCookie == name)
        GetInVarObj<TInvarCookie>(actKc, *this).SetValue(arr, val);
    else
        throw TInnerVarException(actKc.GetCurrLineID(), __FUNCTION__, (boost::format(this->getHint("Don_t_exists_inner_variable")) % name % actKc.GetCurrLineID()).str(), *this);
}
