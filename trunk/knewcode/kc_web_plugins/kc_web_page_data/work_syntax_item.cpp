#include "std.h"
#include "work_syntax_item.h"
#include "expr_item/expr_item_if.h"
using namespace syntax_item;

#include "syntax_item/one_level/html_item.h"
#include "syntax_item/one_level/load_item.h"
#include "syntax_item/one_level/var_def_item.h"
#include "syntax_item/one_level/var_ass_item.h"
#include "syntax_item/one_level/invar_ass_item.h"
#include "syntax_item/one_level/expr_item.h"
#include "syntax_item/one_level/print_item.h"
#include "syntax_item/one_level/if_item.h"
#include "syntax_item/one_level/else_if_item.h"
#include "syntax_item/one_level/else_item.h"
#include "syntax_item/one_level/end_if_item.h"
#include "syntax_item/one_level/while_item.h"
#include "syntax_item/one_level/break_item.h"
#include "syntax_item/one_level/continue_item.h"
#include "syntax_item/one_level/end_while_item.h"
#include "syntax_item/one_level/exit_item.h"
using namespace syntax_item::one_level;

#include "syntax_item/two_level/interface_item.h"
#include "syntax_item/two_level/include_item.h"
#include "syntax_item/two_level/event_item.h"
using namespace syntax_item::two_level;

////////////////////////////////////////////////////////////////////////////////
// CSyntaxItemWork类
CSyntaxItemWork::CSyntaxItemWork(IWebPageData& pd) : m_WebPageData(pd)
{
}

// 获取语法项
ISyntaxItem* CSyntaxItemWork::GetSyntaxItem(int i)
{
    ISyntaxItem* result = nullptr;
    if ((int)m_SyntaxItemList.size() > i && i >= 0) result = m_SyntaxItemList[i].get();
    return result;
}

// 添加语法项
inline void CSyntaxItemWork::AddSyntaxItem(ISyntaxItem* pSyn)
{
    TSyntaxItemPtr SynPtr(pSyn);
    pSyn->ParseItem();
    m_SyntaxItemList.push_back(SynPtr);
    cout << endl << "[" << pSyn->GetKeychar() << "]" << endl;
}
template<typename SYNTAX>
inline SYNTAX* CSyntaxItemWork::AddSyntaxItem(void)
{
    SYNTAX* pSyn = new SYNTAX(m_WebPageData, m_SyntaxItemList.size());
    this->AddSyntaxItem(pSyn);
    return pSyn;
}
template<typename SYNTAX, typename DATA>
inline SYNTAX* CSyntaxItemWork::AddSyntaxItem(const DATA& db)
{
    SYNTAX* pSyn = new SYNTAX(m_WebPageData, m_SyntaxItemList.size(), db);
    this->AddSyntaxItem(pSyn);
    return pSyn;
}

// 添加HTML
void CSyntaxItemWork::AddHtmlTextSyntax(string html)
{
    AddSyntaxItem<CHtmlItem>(html);
}

// 添加include
void CSyntaxItemWork::AddIncludeSyntax(const TKcWebExpr& path)
{
    AddSyntaxItem<CIncludeItem>(path);
}

// 添加变量定义
void CSyntaxItemWork::AddVarDefSyntax(const TKcWebVarDef& vd)
{
    AddSyntaxItem<CVarDefItem>(vd);
}

// 添加变量赋值
void CSyntaxItemWork::AddVarAssignSyntax(const TKcWebVarAss& va)
{
    AddSyntaxItem<CVarAssItem>(va);
}

// 添加变量赋值
void CSyntaxItemWork::AddInnerVarAssSyntax(const TKcWebInnerVarAss& va)
{
    AddSyntaxItem<CInnerVarAssItem>(va);
}

// 添加表达式运算
void CSyntaxItemWork::AddExprSyntax(const TKcWebExpr& exp)
{
    AddSyntaxItem<CExprItem>(exp);
}

// 添加事件
void CSyntaxItemWork::AddEventDefSyntax(const TKcEventDef& ev)
{
    AddSyntaxItem<CEventItem>(ev);
}

// 添加函数库（load、interface等）
void CSyntaxItemWork::AddLoadSyntax(const TKcWebLoadSect& load)
{
    if (TKcWebLoad::fltLoad == load.LoadLib.LibType)
        AddSyntaxItem<CLoadItem>(load);
    else
        AddSyntaxItem<CInterfaceItem>(load);
}

// 添加输出语句
void CSyntaxItemWork::AddPrintSection(const TKcWebExpr& exp)
{
    AddSyntaxItem<CPrintItem>(exp);
}

// 添加条件语句
void CSyntaxItemWork::AddIfSection(const TKcWebExpr& exp)
{
    CIfItem* pSyn = AddSyntaxItem<CIfItem>(exp);
    m_SyntaxItemStack.push_back(pSyn);
}
void CSyntaxItemWork::AddElseIfSection(const TKcWebExpr& exp)
{
    CElseIfItem* pSyn = AddSyntaxItem<CElseIfItem>(exp);
    // 配对if、else-if语句
    CSyntaxItemImpl* pNext = nullptr;
    if (m_SyntaxItemStack.size() > 0 && (pNext = dynamic_cast<CSyntaxItemImpl*>(*m_SyntaxItemStack.rbegin())) != nullptr
            && (dynamic_cast<CIfItem*>(pNext) != nullptr || dynamic_cast<CElseIfItem*>(pNext) != nullptr))
    {
        pNext->SetNextRelated(*pSyn);
        m_SyntaxItemStack.pop_back();
        m_SyntaxItemStack.push_back(pSyn);
    }
    else
        throw TWebPageDataFactoryException(pSyn->GetLineID(), __FUNCTION__, "There are no corresponding statement [else-if], at line " + lexical_cast<string>(pSyn->GetLineID()) + ", in " + __FUNCTION__, m_WebPageData.GetFactory());
}
void CSyntaxItemWork::AddElseSection(void)
{
    CElseItem* pSyn = AddSyntaxItem<CElseItem>();
    // 配对if、else-if语句
    CSyntaxItemImpl* pNext = nullptr;
    if (m_SyntaxItemStack.size() > 0 && (pNext = dynamic_cast<CSyntaxItemImpl*>(*m_SyntaxItemStack.rbegin())) != nullptr
            && (dynamic_cast<CIfItem*>(pNext) != nullptr || dynamic_cast<CElseIfItem*>(pNext) != nullptr))
    {
        pNext->SetNextRelated(*pSyn);
        m_SyntaxItemStack.pop_back();
        m_SyntaxItemStack.push_back(pSyn);
    }
    else
        throw TWebPageDataFactoryException(pSyn->GetLineID(), __FUNCTION__, "There are no corresponding statement [else], at line " + lexical_cast<string>(pSyn->GetLineID()) + ", in " + __FUNCTION__, m_WebPageData.GetFactory());
}
void CSyntaxItemWork::AddEndIfSection(void)
{
    CEndIfItem* pSyn = AddSyntaxItem<CEndIfItem>();
    // 配对if、else-if、else语句
    CSyntaxItemImpl* pNext = nullptr;
    if (m_SyntaxItemStack.size() > 0 && (pNext = dynamic_cast<CSyntaxItemImpl*>(*m_SyntaxItemStack.rbegin())) != nullptr
            && (dynamic_cast<CIfItem*>(pNext) != nullptr || dynamic_cast<CElseIfItem*>(pNext) != nullptr || dynamic_cast<CElseItem*>(pNext) != nullptr))
    {
        pNext->SetNextRelated(*pSyn);
        m_SyntaxItemStack.pop_back();
    }
    else
        throw TWebPageDataFactoryException(pSyn->GetLineID(), __FUNCTION__, "There are no corresponding statement [end-if], at line " + lexical_cast<string>(pSyn->GetLineID()) + ", in " + __FUNCTION__, m_WebPageData.GetFactory());
}

// 添加循环语句
void CSyntaxItemWork::AddWhileSection(const TKcWebExpr& exp)
{
    CWhileItem* pSyn = AddSyntaxItem<CWhileItem>(exp);
    m_SyntaxItemStack.push_back(pSyn);
}
void CSyntaxItemWork::AddBreakSection(const TKcWebExpr& exp)
{
    CBreakItem* pSyn = AddSyntaxItem<CBreakItem>(exp);
    // 配对while语句
    CWhileItem* pSynWhile = nullptr;
    if (m_SyntaxItemStack.size() > 0 && (pSynWhile = dynamic_cast<CWhileItem*>(*m_SyntaxItemStack.rbegin())) != nullptr)
        pSyn->SetNextRelated(*pSynWhile);
    else
        throw TWebPageDataFactoryException(pSyn->GetLineID(), __FUNCTION__, "There are no corresponding [while] statement, at line " + lexical_cast<string>(pSyn->GetLineID()) + ", in " + __FUNCTION__, m_WebPageData.GetFactory());
}
void CSyntaxItemWork::AddContinueSection(const TKcWebExpr& exp)
{
    CContinueItem* pSyn = AddSyntaxItem<CContinueItem>(exp);
    // 配对while语句
    CWhileItem* pSynWhile = nullptr;
    if (m_SyntaxItemStack.size() > 0 && (pSynWhile = dynamic_cast<CWhileItem*>(*m_SyntaxItemStack.rbegin())) != nullptr)
        pSyn->SetNextRelated(*pSynWhile);
    else
        throw TWebPageDataFactoryException(pSyn->GetLineID(), __FUNCTION__, "There are no corresponding [while] statement, at line " + lexical_cast<string>(pSyn->GetLineID()) + ", in " + __FUNCTION__, m_WebPageData.GetFactory());
}
void CSyntaxItemWork::AddEndWhileSection(void)
{
    CEndWhileItem* pSyn = AddSyntaxItem<CEndWhileItem>();
    // 配对while语句
    CWhileItem* pSynWhile = nullptr;
    if (m_SyntaxItemStack.size() > 0 && (pSynWhile = dynamic_cast<CWhileItem*>(*m_SyntaxItemStack.rbegin())) != nullptr)
    {
        pSynWhile->SetNextRelated(*pSyn);
        pSyn->SetNextRelated(*pSynWhile);
        m_SyntaxItemStack.pop_back();
    }
    else
        throw TWebPageDataFactoryException(pSyn->GetLineID(), __FUNCTION__, "There are no corresponding [while] statement, at line " + lexical_cast<string>(pSyn->GetLineID()) + ", in " + __FUNCTION__, m_WebPageData.GetFactory());
}

// 退出语句
void CSyntaxItemWork::AddExitSection(const TKcWebArgms& arg)
{
    AddSyntaxItem<CExitItem>(arg);
}
