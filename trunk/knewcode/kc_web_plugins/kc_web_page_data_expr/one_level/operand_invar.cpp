#include "../std.h"
#include "operand_invar.h"
#include "../work_expr_tree.h"

////////////////////////////////////////////////////////////////////////////////
// TInnerVarNode类
KC::one_level::TInnerVarNode::TInnerVarNode(IExprTreeWork& wk, IWebPageData& pd, const TKcInnerVarSyn& syn)
    : TOperandNode(wk, pd, syn), m_syn(syn)
{
}

// 获取变量的值
boost::any KC::one_level::TInnerVarNode::GetValue(IKCActionData& act)
{
    // 变量名
    string sVarName = m_syn.GetVal<0>();
    // 下标值
    std::vector<boost::any> arrList;
    this->GetArray(arrList, act);
    // 调用应用
    IInnerVar& inVar = dynamic_cast<CExprTreeWork&>(m_work).getInnerVarInf();
    return inVar.GetValue(act, sVarName, arrList);
}

// 设置变量的值
void KC::one_level::TInnerVarNode::SetValue(IKCActionData& act, boost::any val)
{
    // 变量名
    string sVarName = m_syn.GetVal<0>();
    // 下标值
    std::vector<boost::any> arrList;
    this->GetArray(arrList, act);
    // 调用应用
    IInnerVar& inVar = dynamic_cast<CExprTreeWork&>(m_work).getInnerVarInf();
    inVar.SetValue(act, sVarName, arrList, val);
}

// 获取下标值
void KC::one_level::TInnerVarNode::GetArray(std::vector<boost::any>& arrList, IKCActionData& act)
{
    for (int i = 0; i < (int)m_syn.GetVal<1>().ValList->size(); ++i)
    {
        TOperandNode *arrExpr = dynamic_cast<TOperandNode*>((*m_syn.GetVal<1>().ValList)[i].exprTreeNodePtr.get());
        if (nullptr == arrExpr)
            throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_operand_can_only_be_specific_type")) % i % this->GetLineID()).str(), m_work);
        arrList.push_back(arrExpr->GetValue(act));
    }
    if (arrList.empty())
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, m_work.getHint("The_array_index_is_empty_") + lexical_cast<string>(this->GetLineID()), m_work);
}
