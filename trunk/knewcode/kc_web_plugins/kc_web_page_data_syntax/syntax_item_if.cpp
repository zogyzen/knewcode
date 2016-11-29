#include "std.h"
#include "syntax_item_if.h"
#include "work_syntax_item.h"

////////////////////////////////////////////////////////////////////////////////
// CSyntaxItemImpl类
KC::CSyntaxItemImpl::CSyntaxItemImpl(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcSynBaseClass& syn)
    : m_work(work), m_pd(pd), m_Index(i), m_pBeginPtr(syn.GetBeginPtr()), m_pEndPtr(syn.GetEndPtr()), m_LineID(pd.GetLineID(syn))
{
}

// 得到序号
int KC::CSyntaxItemImpl::GetIndex(void) const
{
    return m_Index;
}

// 得到行号
int KC::CSyntaxItemImpl::GetLineID(void) const
{
    return m_LineID;
}

// 获取语法起始位置指针
const char* KC::CSyntaxItemImpl::GetBeginPtr(void) const
{
    return m_pBeginPtr;
}
const char* KC::CSyntaxItemImpl::GetEndPtr(void) const
{
    return m_pEndPtr;
}

// 获取语法内容
string KC::CSyntaxItemImpl::GetSyntaxContent(void) const
{
    if (nullptr == m_pBeginPtr)
        return this->GetKeychar();
    else if (nullptr == m_pEndPtr)
        return string(m_pBeginPtr, m_pBeginPtr + std::min(m_pd.GetTextBuffer().last - m_pBeginPtr, 30));
    else
        return string(m_pBeginPtr, m_pBeginPtr + std::min(std::max(m_pEndPtr - m_pBeginPtr, 10), std::min(m_pd.GetTextBuffer().last - m_pBeginPtr, 200)));
}

// 解析数据
void KC::CSyntaxItemImpl::ParseItem(void)
{
}

// 相关语法项
int KC::CSyntaxItemImpl::GetNextIndex(IKCActionData&)
{
    return m_Index + 1;
}

// 处理页数据
void KC::CSyntaxItemImpl::ActionItem(IKCActionData& act)
{
    this->Action(act);
}

// 处理页数据
void KC::CSyntaxItemImpl::Action(IKCActionData&)
{
}

// 非顺序语句的条件栈——压栈
void KC::CSyntaxItemImpl::PushConditionStatck(TConditionStatck& condStatck, string key, bool cond)
{
    this->PushConditionStatck(condStatck, TConditionItem(key, cond));
}
void KC::CSyntaxItemImpl::PushConditionStatck(TConditionStatck& condStatck, const TConditionItem& cond)
{
    condStatck.push_back(cond);
}

// 非顺序语句的条件栈——弹栈
bool KC::CSyntaxItemImpl::PopConditionStatck(TConditionStatck& condStatck, TConditionItem& cond)
{
    bool bRes = !condStatck.empty();
    if (bRes)
    {
        cond = *condStatck.rbegin();
        condStatck.pop_back();
    }
    return bRes;
}

// 设置相关语法项
void KC::CSyntaxItemImpl::SetNextRelated(CSyntaxItemImpl& syn)
{
    m_next = &syn;
}

// 得到相关语法项
KC::CSyntaxItemImpl* KC::CSyntaxItemImpl::GetNextRelated(void)
{
    return m_next;
}

// 获取标识符的应用
IIDNameItemWork& KC::CSyntaxItemImpl::getIIDNameInf(void)
{
    CSyntaxItemWork& work = dynamic_cast<CSyntaxItemWork&>(m_work);
    return work.getIIDNameInf();
}

// 获取页面工厂的应用
IWebPageDataFactory& CSyntaxItemImpl::getPageFactoryRef(void)
{
    CSyntaxItemWork& work = dynamic_cast<CSyntaxItemWork&>(m_work);
    return work.getPageFactoryRef();
}
