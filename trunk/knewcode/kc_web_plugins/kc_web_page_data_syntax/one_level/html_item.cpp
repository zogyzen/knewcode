#include "../std.h"
#include "html_item.h"

////////////////////////////////////////////////////////////////////////////////
// CHtmlItem类
KC::one_level::CHtmlItem::CHtmlItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcHtmlSyn& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
    m_pEndPtr = nullptr;
    // cout << "<html>" << m_syn.GetVal<0>() << "</html>" << endl;
}

// 获取语法关键字
string KC::one_level::CHtmlItem::GetKeychar(void) const
{
    return "HTML";
}

// 语法类型转换
const TKcHtmlSyn& KC::one_level::CHtmlItem::DynCast(const TKcSynBaseClass& syn, IWebPageData& pd)
{
    const TKcHtmlSyn& result = dynamic_cast<const TKcHtmlSyn&>(syn);
    result.SynPosAttrList->insert(result.SynPosAttrList->begin(), pd.GetPrevPos());
    return result;
}

// 处理页数据
void KC::one_level::CHtmlItem::Action(IKCActionData& act)
{
    act.GetRequestRespond().OutputHTML(m_syn.GetVal<0>().c_str());
}
