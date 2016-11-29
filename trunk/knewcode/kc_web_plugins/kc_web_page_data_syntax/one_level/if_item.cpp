#include "../std.h"
#include "if_item.h"

////////////////////////////////////////////////////////////////////////////////
// CIfItem类
KC::one_level::CIfItem::CIfItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcIfSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CIfItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordIf;
}

// 语法类型转换
const TKcIfSent& KC::one_level::CIfItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcIfSent&>(syn);
}

// 相关语法项
int KC::one_level::CIfItem::GetNextIndex(IKCActionData& act)
{
    TConditionStatck& condStatck = act.ConditionStatck();
    // 下条对应语句
    if (nullptr == this->m_next)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Next_statements_do_not_correspond")) % this->GetKeychar() % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
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
