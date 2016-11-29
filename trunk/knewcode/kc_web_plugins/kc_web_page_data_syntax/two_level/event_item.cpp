#include "../std.h"
#include "event_item.h"

////////////////////////////////////////////////////////////////////////////////
// CEventItem类
KC::two_level::CEventItem::CEventItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcEventDef& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::two_level::CEventItem::GetKeychar(void) const
{
    return Keychar::two_level::g_KeywordEvent;
}

// 语法类型转换
const TKcEventDef& KC::two_level::CEventItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcEventDef&>(syn);
}

// 处理页数据
void KC::two_level::CEventItem::Action(IKCActionData& act)
{
    //act.EventWork().AddEvent(m_event);
}
