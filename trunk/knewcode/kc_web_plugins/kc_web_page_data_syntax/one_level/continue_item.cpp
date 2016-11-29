#include "../std.h"
#include "continue_item.h"

////////////////////////////////////////////////////////////////////////////////
// CContinueItem类
KC::one_level::CContinueItem::CContinueItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcContinueSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CContinueItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordContinue;
}

// 语法类型转换
const TKcContinueSent& KC::one_level::CContinueItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcContinueSent&>(syn);
}

// 相关语法项
int KC::one_level::CContinueItem::GetNextIndex(IKCActionData& act)
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
        // 作废上个栈条件
        TConditionItem cond;
        this->PopConditionStatck(condStatck, cond);
        // 跳转
        return this->m_next->GetIndex();
    }
    // 条件为假：顺序到下条语句
    else
        return CSyntaxItemImpl::GetNextIndex(act);
}
