#include "../std.h"
#include "end_if_item.h"

////////////////////////////////////////////////////////////////////////////////
// CEndIfItem类
KC::one_level::CEndIfItem::CEndIfItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcEndIfSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CEndIfItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordEndIf;
}

// 语法类型转换
const TKcEndIfSent& KC::one_level::CEndIfItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcEndIfSent&>(syn);
}

// 相关语法项
int KC::one_level::CEndIfItem::GetNextIndex(IKCActionData& act)
{
    TConditionStatck& condStatck = act.ConditionStatck();
    // 上条对应条件弹栈
    TConditionItem cond;
    if (!this->PopConditionStatck(condStatck, cond) || (cond.keychar != g_KeywordIf && cond.keychar != g_KeywordElseIf && cond.keychar != g_KeywordElse))
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Statement_does_not_correspond_on")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 顺序下条语句
    return CSyntaxItemImpl::GetNextIndex(act);
}
