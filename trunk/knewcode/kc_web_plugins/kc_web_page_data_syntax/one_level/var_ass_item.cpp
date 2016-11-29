#include "../std.h"
#include "var_ass_item.h"

////////////////////////////////////////////////////////////////////////////////
// CVarAssItem类
KC::one_level::CVarAssItem::CVarAssItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcVarAssDef& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CVarAssItem::GetKeychar(void) const
{
    return "VARIABLE_ASSIGN";
}

// 语法类型转换
const TKcVarAssDef& KC::one_level::CVarAssItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    const TKcVarAssDef& result = dynamic_cast<const TKcVarAssDef&>(syn);
    result.SynPosAttrList->insert(result.SynPosAttrList->begin(), result.GetVal<0>().SynPosAttrList->front());
    return result;
}

// 处理页数据
void KC::one_level::CVarAssItem::Action(IKCActionData& act)
{
    // 变量名
    string sVarName = m_syn.GetVal<0>().GetVal<0>();
    // 变量的值
    IOperandNode *valExpr = dynamic_cast<IOperandNode*>(m_syn.GetVal<1>().exprTreeNodePtr.get());
    if (nullptr == valExpr)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_value_of_the_variable_is_not_specified")) % sVarName % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    boost::any varVal = valExpr->GetValue(act);
    // 变量标识符
    IIDNameItem& idName = act.GetIDName(sVarName);
    IVarItemBase* varName = dynamic_cast<IVarItemBase*>(&idName);
    if (nullptr == varName)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Identifiers_are_not_a_variable")) % sVarName % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    varName->SetVarValue(varVal);
}
