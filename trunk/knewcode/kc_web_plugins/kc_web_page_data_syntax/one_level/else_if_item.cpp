#include "../std.h"
#include "else_if_item.h"

////////////////////////////////////////////////////////////////////////////////
// CElseIfItem类
KC::one_level::CElseIfItem::CElseIfItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcElseIfSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CElseIfItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordElseIf;
}

// 语法类型转换
const TKcElseIfSent& KC::one_level::CElseIfItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcElseIfSent&>(syn);
}

// 相关语法项
int KC::one_level::CElseIfItem::GetNextIndex(IKCActionData& act)
{
    TConditionStatck& condStatck = act.ConditionStatck();
    // 下条对应语句
    if (nullptr == this->m_next)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Next_statements_do_not_correspond")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 上条对应条件弹栈
    TConditionItem cond;
    if (!this->PopConditionStatck(condStatck, cond) || (cond.keychar != g_KeywordIf && cond.keychar != g_KeywordElseIf))
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Statement_does_not_correspond_on")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 上条语句条件为真：跳转到下个对应的语句
    if (cond.cond)
    {
        this->PushConditionStatck(condStatck, cond);
        return this->m_next->GetIndex();
    }
    // 获取条件表达式的值
    bool bCond = this->GetSynCondValue(act, m_syn);
    // 条件压栈
    this->PushConditionStatck(condStatck, this->GetKeychar(), bCond);
    // 条件为真：顺序到下条语句
    if (bCond)
        return CSyntaxItemImpl::GetNextIndex(act);
    // 条件为假：跳转到下个对应的语句
    else
        return this->m_next->GetIndex();
}
