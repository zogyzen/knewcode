#include "../std.h"
#include "invar_ass_item.h"

////////////////////////////////////////////////////////////////////////////////
// CInnerVarAssItem类
KC::one_level::CInnerVarAssItem::CInnerVarAssItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcInnerVarAssDef& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CInnerVarAssItem::GetKeychar(void) const
{
    return "INNER_VARIABLE_ASSIGN";
}

// 语法类型转换
const TKcInnerVarAssDef& KC::one_level::CInnerVarAssItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    const TKcInnerVarAssDef& result = dynamic_cast<const TKcInnerVarAssDef&>(syn);
    result.SynPosAttrList->insert(result.SynPosAttrList->begin(), result.GetVal<0>().SynPosAttrList->front());
    return result;
}

// 处理页数据
void KC::one_level::CInnerVarAssItem::Action(IKCActionData& act)
{
    // 变量名
    string sVarName = m_syn.GetVal<0>().GetVal<0>();
    // 下标值
    TAnyTypeValList arrAnyList;
    for (auto exp: *m_syn.GetVal<0>().GetVal<1>().ValList.get())
        arrAnyList.push_back(act.GetExprValue(exp));
    // 变量的值
    boost::any valVar = act.GetExprValue(m_syn.GetVal<1>());
    // 设置内部变量值
    act.SetInnerVarValue(sVarName, arrAnyList, valVar);
}
