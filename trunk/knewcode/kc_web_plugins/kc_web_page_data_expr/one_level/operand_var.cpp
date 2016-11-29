#include "../std.h"
#include "operand_var.h"

////////////////////////////////////////////////////////////////////////////////
// TVarNode类
KC::one_level::TVarNode::TVarNode(IExprTreeWork& wk, IWebPageData& pd, const TKcVarSyn& syn)
    : TOperandNode(wk, pd, syn), m_syn(syn)
{
}

// 获取操作数的值
boost::any KC::one_level::TVarNode::GetValue(IKCActionData& act)
{
    return this->GetVarIf(act).GetVarValue();
}

// 设置变量的值
void KC::one_level::TVarNode::SetValue(IKCActionData& act, boost::any val)
{
    this->GetVarIf(act).SetVarValue(val);
}

// 获取变量接口
IVarItemBase& KC::one_level::TVarNode::GetVarIf(IKCActionData& act)
{
    // 得到变量
    IVarItemBase* pVar = dynamic_cast<IVarItemBase*>(&act.GetIDName(m_syn.GetVal<0>()));
    if (nullptr == pVar)
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Don_t_exists_variable")) % m_syn.GetVal<0>() % this->GetLineID()).str(), m_work);
    return *pVar;
}
