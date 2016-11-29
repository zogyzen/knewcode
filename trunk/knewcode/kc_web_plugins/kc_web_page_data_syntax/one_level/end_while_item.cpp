#include "../std.h"
#include "end_while_item.h"

////////////////////////////////////////////////////////////////////////////////
// CEndWhileItem类
KC::one_level::CEndWhileItem::CEndWhileItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcEndWhileSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CEndWhileItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordEndWhile;
}

// 语法类型转换
const TKcEndWhileSent& KC::one_level::CEndWhileItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcEndWhileSent&>(syn);
}

// 相关语法项
int KC::one_level::CEndWhileItem::GetNextIndex(IKCActionData& act)
{
    TConditionStatck& condStatck = act.ConditionStatck();
    // 下条对应语句
    if (nullptr == this->m_next)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Next_statements_do_not_correspond")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 获取条件表达式的值
    bool bCond = this->GetSynCondValue(act, m_syn, false);
    // 上条对应条件弹栈
    TConditionItem cond;
    if (!this->PopConditionStatck(condStatck, cond) || cond.keychar != g_KeywordWhile)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Statement_does_not_correspond_on")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 上条语句条件为真，并且退出条件为假：跳转到下个对应的语句
    if (cond.cond && !bCond)
        return this->m_next->GetIndex();
    // 否则退出循环：顺序到下条语句
    else
        return CSyntaxItemImpl::GetNextIndex(act);
}
