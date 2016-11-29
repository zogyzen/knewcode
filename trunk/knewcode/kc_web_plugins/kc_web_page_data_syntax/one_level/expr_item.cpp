#include "../std.h"
#include "expr_item.h"

////////////////////////////////////////////////////////////////////////////////
// CExprItem类
KC::one_level::CExprItem::CExprItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcExprWorkDef& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CExprItem::GetKeychar(void) const
{
    return "EXPRESSION";
}

// 语法类型转换
const TKcExprWorkDef& KC::one_level::CExprItem::DynCast(const TKcSynBaseClass& syn, IWebPageData& pd)
{
    const TKcExprWorkDef& result = dynamic_cast<const TKcExprWorkDef&>(syn);
    result.SynPosAttrList->insert(result.SynPosAttrList->begin(), pd.GetPrevPos());
    return result;
}

// 处理页数据
void KC::one_level::CExprItem::Action(IKCActionData& act)
{
    act.GetExprValue(m_syn.GetVal<0>());
}
