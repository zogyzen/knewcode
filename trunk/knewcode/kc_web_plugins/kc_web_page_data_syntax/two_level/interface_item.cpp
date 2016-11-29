#include "../std.h"
#include "interface_item.h"

////////////////////////////////////////////////////////////////////////////////
// CInterfaceItem类
KC::two_level::CInterfaceItem::CInterfaceItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcInfFullFL& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::two_level::CInterfaceItem::GetKeychar(void) const
{
    return Keychar::two_level::g_KeywordInterface;
}

// 处理页数据
void KC::two_level::CInterfaceItem::Action(IKCActionData& act)
{
    act.AddKcInfFullFL(m_syn);
}

// 语法类型转换
const TKcInfFullFL& KC::two_level::CInterfaceItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    const TKcInfFullFL& result = dynamic_cast<const TKcInfFullFL&>(syn);
    result.SynPosAttrList->insert(result.SynPosAttrList->begin(), result.GetVal<0>().SynPosAttrList->front());
    result.SynPosAttrList->push_back(result.GetVal<0>().SynPosAttrList->back());
    return result;
}
