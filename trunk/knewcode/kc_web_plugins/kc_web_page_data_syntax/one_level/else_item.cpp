#include "../std.h"
#include "else_item.h"

////////////////////////////////////////////////////////////////////////////////
// CElseItem类
KC::one_level::CElseItem::CElseItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcElseSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CElseItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordElse;
}

// 语法类型转换
const TKcElseSent& KC::one_level::CElseItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcElseSent&>(syn);
}

// 相关语法项
int KC::one_level::CElseItem::GetNextIndex(IKCActionData& act)
{
    TConditionStatck& condStatck = act.ConditionStatck();
    // 下条对应语句
    if (nullptr == this->m_next)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Next_statements_do_not_correspond")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 上条对应条件弹栈
    TConditionItem cond;
    if (!this->PopConditionStatck(condStatck, cond) || (cond.keychar != g_KeywordIf && cond.keychar != g_KeywordElseIf))
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Statement_does_not_correspond_on")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 条件再压栈
    this->PushConditionStatck(condStatck, cond);
    // 上条语句条件为真：跳转到下个对应的语句
    if (cond.cond)
        return this->m_next->GetIndex();
    // 上条语句条件为假：顺序下条语句
    else
        return CSyntaxItemImpl::GetNextIndex(act);
}
