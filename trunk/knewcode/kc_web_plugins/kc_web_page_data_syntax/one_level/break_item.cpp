#include "../std.h"
#include "break_item.h"

////////////////////////////////////////////////////////////////////////////////
// CBreakItem类
KC::one_level::CBreakItem::CBreakItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcBreakSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CBreakItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordBreak;
}

// 语法类型转换
const TKcBreakSent& KC::one_level::CBreakItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcBreakSent&>(syn);
}

// 相关语法项
int KC::one_level::CBreakItem::GetNextIndex(IKCActionData& act)
{
    TConditionStatck& condStatck = act.ConditionStatck();
    // 下条对应语句
    if (nullptr == this->m_next)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Next_statements_do_not_correspond")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 获取条件表达式的值
    bool bCond = this->GetSynCondValue(act, m_syn);
    // 条件为真：跳转到下个对应的语句
    if (bCond)
    {
        // 下条对应语句
        if (nullptr == this->m_next || this->m_next->GetNextRelated() == nullptr)
            throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Next_statements_do_not_correspond")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
        // 上条对应条件弹栈
        TConditionItem cond;
        if (!this->PopConditionStatck(condStatck, cond))
            throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Statement_does_not_correspond_on")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
        // 条件改为假后，再压栈
        cond.cond = false;
        this->PushConditionStatck(condStatck, cond);
        // 跳转到下个对应语句
        return this->m_next->GetNextRelated()->GetIndex();
    }
    // 条件为假：顺序到下条语句
    else
        return CSyntaxItemImpl::GetNextIndex(act);
}
