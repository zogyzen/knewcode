#include "std.h"
#include "work_syntax_item.h"
#include "syntax_item_if.h"

#include "one_level/html_item.h"
#include "one_level/load_item.h"
#include "one_level/var_def_item.h"
#include "one_level/var_ass_item.h"
#include "one_level/invar_ass_item.h"
#include "one_level/expr_item.h"
#include "one_level/print_item.h"
#include "one_level/exec_item.h"
#include "one_level/if_item.h"
#include "one_level/else_if_item.h"
#include "one_level/else_item.h"
#include "one_level/end_if_item.h"
#include "one_level/while_item.h"
#include "one_level/break_item.h"
#include "one_level/continue_item.h"
#include "one_level/end_while_item.h"
#include "one_level/exit_item.h"
using namespace one_level;

#include "two_level/interface_item.h"
#include "two_level/include_item.h"
#include "two_level/delay_item.h"
#include "two_level/event_item.h"
using namespace two_level;

////////////////////////////////////////////////////////////////////////////////
// CSyntaxItemWork类
CSyntaxItemWork::CSyntaxItemWork(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle)
{
}

// 得到服务特征码
const char* CSyntaxItemWork::getGUID(void) const
{
    return c_SyntaxWorkSrvGUID;
}

// 对应的模块
const IBundle& CSyntaxItemWork::getBundle(void) const
{
    return m_bundle;
}

// 添加语法项
template<typename SYNTAX, typename DATA>
inline SYNTAX* CSyntaxItemWork::AddSyntaxItem(IWebPageData& pd, int index, const DATA& db)
{
    SYNTAX* pSyn = new SYNTAX(*this, pd, index, db);
    pSyn->ParseItem();
    // cout << endl << "[" << pSyn->GetKeychar() << "]" << endl;
    return pSyn;
}

// 添加语法项
TSyntaxItemPtr CSyntaxItemWork::NewSyntaxItem(const TKcSynBaseClass& syn, IWebPageData& pd, int index, TSyntaxItemStack& SynStack)
{
    ISyntaxItem* pResult = nullptr;
    // 条件语句
    if (syn.GetID() == c_SynSysSentID_If && dynamic_cast<const TKcIfSent*>(&syn) != nullptr)
        pResult = this->NewIfSection(pd, index, CIfItem::DynCast(syn, pd), SynStack);
    else if (syn.GetID() == c_SynSysSentID_ElseIf && dynamic_cast<const TKcElseIfSent*>(&syn) != nullptr)
        pResult = this->NewElseIfSection(pd, index, CElseIfItem::DynCast(syn, pd), SynStack);
    else if (syn.GetID() == c_SynSysSentID_Else && dynamic_cast<const TKcElseSent*>(&syn) != nullptr)
        pResult = this->NewElseSection(pd, index, CElseItem::DynCast(syn, pd), SynStack);
    else if (syn.GetID() == c_SynSysSentID_EndIf && dynamic_cast<const TKcEndIfSent*>(&syn) != nullptr)
        pResult = this->NewEndIfSection(pd, index, CEndIfItem::DynCast(syn, pd), SynStack);
    // 循环语句
    else if (syn.GetID() == c_SynSysSentID_While && dynamic_cast<const TKcWhileSent*>(&syn) != nullptr)
        pResult = this->NewWhileSection(pd, index, CWhileItem::DynCast(syn, pd), SynStack);
    else if (syn.GetID() == c_SynSysSentID_Break && dynamic_cast<const TKcBreakSent*>(&syn) != nullptr)
        pResult = this->NewBreakSection(pd, index, CBreakItem::DynCast(syn, pd), SynStack);
    else if (syn.GetID() == c_SynSysSentID_Continue && dynamic_cast<const TKcContinueSent*>(&syn) != nullptr)
        pResult = this->NewContinueSection(pd, index, CContinueItem::DynCast(syn, pd), SynStack);
    else if (syn.GetID() == c_SynSysSentID_EndWhile && dynamic_cast<const TKcEndWhileSent*>(&syn) != nullptr)
        pResult = this->NewEndWhileSection(pd, index, CEndWhileItem::DynCast(syn, pd), SynStack);
    // 其他语法项
    else
        return NewSyntaxItem(syn, pd, index);
    // 创建语法项失败
    if (nullptr == pResult)
        throw TSyntaxWorkSrvException(pd.GetLineID(syn), __FUNCTION__, (boost::format(this->getHint("Can_t_create_Syntax_Item")) % syn.GetID() % pd.GetLineID(syn)).str(), *this, syn.GetBeginPtr(), syn.GetEndPtr());
    // 返回
    return TSyntaxItemPtr(pResult);
}

// 创建语法项（非结构语句）
TSyntaxItemPtr CSyntaxItemWork::NewSyntaxItem(const TKcSynBaseClass& syn, IWebPageData& pd, int index)
{
    ISyntaxItem* pResult = nullptr;
    // HTML内容
    if (syn.GetID() == c_SynSysBaseID_Html && dynamic_cast<const TKcHtmlSyn*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CHtmlItem>(pd, index, CHtmlItem::DynCast(syn, pd));
    // 变量定义
    else if (syn.GetID() == c_SynSysDefID_Var && dynamic_cast<const TKcVarDef*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CVarDefItem>(pd, index, CVarDefItem::DynCast(syn, pd));
    // 变量赋值
    else if (syn.GetID() == c_SynSysDefID_VarAss && dynamic_cast<const TKcVarAssDef*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CVarAssItem>(pd, index, CVarAssItem::DynCast(syn, pd));
    // 内部变量赋值
    else if (syn.GetID() == c_SynSysDefID_InnerVarAss && dynamic_cast<const TKcInnerVarAssDef*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CInnerVarAssItem>(pd, index, CInnerVarAssItem::DynCast(syn, pd));
    // 表达式运算
    else if (syn.GetID() == c_SynSysDefID_ExprWork && dynamic_cast<const TKcExprWorkDef*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CExprItem>(pd, index, CExprItem::DynCast(syn, pd));
    // 延迟
    else if (syn.GetID() == c_SynSysDefID_Delay && dynamic_cast<const TKcDelayDef*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CDelayItem>(pd, index, CDelayItem::DynCast(syn, pd));
    // 事件
    else if (syn.GetID() == c_SynSysDefID_Event && dynamic_cast<const TKcEventDef*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CEventItem>(pd, index, CEventItem::DynCast(syn, pd));
    // 接口
    else if (syn.GetID() == c_SynSysLibID_InfFull && dynamic_cast<const TKcInfFullFL*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CInterfaceItem>(pd, index, CInterfaceItem::DynCast(syn, pd));
    // 加载
    else if (syn.GetID() == c_SynSysLibID_LoadFull && dynamic_cast<const TKcLoadFullFL*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CLoadItem>(pd, index, CLoadItem::DynCast(syn, pd));
    // 包含
    else if (syn.GetID() == c_SynIncludeID_Single && dynamic_cast<const TKcIncludeCL*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CIncludeItem>(pd, index, CIncludeItem::DynCast(syn, pd));
    // 输出语句
    else if (syn.GetID() == c_SynSysSentID_Print && dynamic_cast<const TKcPrintSent*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CPrintItem>(pd, index, CPrintItem::DynCast(syn, pd));
    // 退出语句
    else if (syn.GetID() == c_SynSysSentID_Exit && dynamic_cast<const TKcExitSent*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CExitItem>(pd, index, CExitItem::DynCast(syn, pd));
    // 执行语句
    else if (syn.GetID() == c_SynSysSentID_Exec && dynamic_cast<const TKcExecSent*>(&syn) != nullptr)
        pResult = AddSyntaxItem<CExecItem>(pd, index, CExecItem::DynCast(syn, pd));
    // 未匹配语法项
    else
        throw TSyntaxWorkSrvException(pd.GetLineID(syn), __FUNCTION__, (boost::format(this->getHint("Unmatch_Syntax_Item")) % syn.GetID() % pd.GetLineID(syn)).str(), *this, syn.GetBeginPtr(), syn.GetEndPtr());
    // 创建语法项失败
    if (nullptr == pResult)
        throw TSyntaxWorkSrvException(pd.GetLineID(syn), __FUNCTION__, (boost::format(this->getHint("Can_t_create_Syntax_Item")) % syn.GetID() % pd.GetLineID(syn)).str(), *this, syn.GetBeginPtr(), syn.GetEndPtr());
    // 返回
    return TSyntaxItemPtr(pResult);
}

// 条件语句
ISyntaxItem* CSyntaxItemWork::NewIfSection(IWebPageData& pd, int index, const TKcIfSent& syn, TSyntaxItemStack& SynStack)
{
    CIfItem* pSyn = AddSyntaxItem<CIfItem>(pd, index, syn);
    SynStack.push_back(pSyn);
    return pSyn;
}
ISyntaxItem* CSyntaxItemWork::NewElseIfSection(IWebPageData& pd, int index, const TKcElseIfSent& syn, TSyntaxItemStack& SynStack)
{
    CElseIfItem* pSyn = AddSyntaxItem<CElseIfItem>(pd, index, syn);
    // 配对if、else-if语句
    CSyntaxItemImpl* pNext = nullptr;
    if (SynStack.size() > 0 && (pNext = dynamic_cast<CSyntaxItemImpl*>(*SynStack.rbegin())) != nullptr
            && (dynamic_cast<CIfItem*>(pNext) != nullptr || dynamic_cast<CElseIfItem*>(pNext) != nullptr))
    {
        pNext->SetNextRelated(*pSyn);
        SynStack.pop_back();
        SynStack.push_back(pSyn);
    }
    else
        throw TSyntaxWorkSrvException(pSyn->GetLineID(), __FUNCTION__, (boost::format(this->getHint("There_are_no_corresponding_statement")) % pSyn->GetKeychar() % pSyn->GetLineID()).str(), *this, pSyn->GetBeginPtr(), pSyn->GetEndPtr());
    return pSyn;
}
ISyntaxItem* CSyntaxItemWork::NewElseSection(IWebPageData& pd, int index, const TKcElseSent& syn, TSyntaxItemStack& SynStack)
{
    CElseItem* pSyn = AddSyntaxItem<CElseItem>(pd, index, syn);
    // 配对if、else-if语句
    CSyntaxItemImpl* pNext = nullptr;
    if (SynStack.size() > 0 && (pNext = dynamic_cast<CSyntaxItemImpl*>(*SynStack.rbegin())) != nullptr
            && (dynamic_cast<CIfItem*>(pNext) != nullptr || dynamic_cast<CElseIfItem*>(pNext) != nullptr))
    {
        pNext->SetNextRelated(*pSyn);
        SynStack.pop_back();
        SynStack.push_back(pSyn);
    }
    else
        throw TSyntaxWorkSrvException(pSyn->GetLineID(), __FUNCTION__, (boost::format(this->getHint("There_are_no_corresponding_statement")) % pSyn->GetKeychar() % pSyn->GetLineID()).str(), *this, pSyn->GetBeginPtr(), pSyn->GetEndPtr());
    return pSyn;
}
ISyntaxItem* CSyntaxItemWork::NewEndIfSection(IWebPageData& pd, int index, const TKcEndIfSent& syn, TSyntaxItemStack& SynStack)
{
    CEndIfItem* pSyn = AddSyntaxItem<CEndIfItem>(pd, index, syn);
    // 配对if、else-if、else语句
    CSyntaxItemImpl* pNext = nullptr;
    if (SynStack.size() > 0 && (pNext = dynamic_cast<CSyntaxItemImpl*>(*SynStack.rbegin())) != nullptr
            && (dynamic_cast<CIfItem*>(pNext) != nullptr || dynamic_cast<CElseIfItem*>(pNext) != nullptr || dynamic_cast<CElseItem*>(pNext) != nullptr))
    {
        pNext->SetNextRelated(*pSyn);
        SynStack.pop_back();
    }
    else
        throw TSyntaxWorkSrvException(pSyn->GetLineID(), __FUNCTION__, (boost::format(this->getHint("There_are_no_corresponding_statement")) % pSyn->GetKeychar() % pSyn->GetLineID()).str(), *this, pSyn->GetBeginPtr(), pSyn->GetEndPtr());
    return pSyn;
}

// 添加循环语句
ISyntaxItem* CSyntaxItemWork::NewWhileSection(IWebPageData& pd, int index, const TKcWhileSent& syn, TSyntaxItemStack& SynStack)
{
    CWhileItem* pSyn = AddSyntaxItem<CWhileItem>(pd, index, syn);
    SynStack.push_back(pSyn);
    return pSyn;
}
ISyntaxItem* CSyntaxItemWork::NewBreakSection(IWebPageData& pd, int index, const TKcBreakSent& syn, TSyntaxItemStack& SynStack)
{
    CBreakItem* pSyn = AddSyntaxItem<CBreakItem>(pd, index, syn);
    // 配对while语句
    CWhileItem* pSynWhile = nullptr;
    if (SynStack.size() > 0 && (pSynWhile = dynamic_cast<CWhileItem*>(*SynStack.rbegin())) != nullptr)
        pSyn->SetNextRelated(*pSynWhile);
    else
        throw TSyntaxWorkSrvException(pSyn->GetLineID(), __FUNCTION__, (boost::format(this->getHint("There_are_no_corresponding_statement")) % pSyn->GetKeychar() % pSyn->GetLineID()).str(), *this, pSyn->GetBeginPtr(), pSyn->GetEndPtr());
    return pSyn;
}
ISyntaxItem* CSyntaxItemWork::NewContinueSection(IWebPageData& pd, int index, const TKcContinueSent& syn, TSyntaxItemStack& SynStack)
{
    CContinueItem* pSyn = AddSyntaxItem<CContinueItem>(pd, index, syn);
    // 配对while语句
    CWhileItem* pSynWhile = nullptr;
    if (SynStack.size() > 0 && (pSynWhile = dynamic_cast<CWhileItem*>(*SynStack.rbegin())) != nullptr)
        pSyn->SetNextRelated(*pSynWhile);
    else
        throw TSyntaxWorkSrvException(pSyn->GetLineID(), __FUNCTION__, (boost::format(this->getHint("There_are_no_corresponding_statement")) % pSyn->GetKeychar() % pSyn->GetLineID()).str(), *this, pSyn->GetBeginPtr(), pSyn->GetEndPtr());
    return pSyn;
}
ISyntaxItem* CSyntaxItemWork::NewEndWhileSection(IWebPageData& pd, int index, const TKcEndWhileSent& syn, TSyntaxItemStack& SynStack)
{
    CEndWhileItem* pSyn = AddSyntaxItem<CEndWhileItem>(pd, index, syn);
    // 配对while语句
    CWhileItem* pSynWhile = nullptr;
    if (SynStack.size() > 0 && (pSynWhile = dynamic_cast<CWhileItem*>(*SynStack.rbegin())) != nullptr)
    {
        pSynWhile->SetNextRelated(*pSyn);
        pSyn->SetNextRelated(*pSynWhile);
        SynStack.pop_back();
    }
    else
        throw TSyntaxWorkSrvException(pSyn->GetLineID(), __FUNCTION__, (boost::format(this->getHint("There_are_no_corresponding_statement")) % pSyn->GetKeychar() % pSyn->GetLineID()).str(), *this, pSyn->GetBeginPtr(), pSyn->GetEndPtr());
    return pSyn;
}

// 获取标识符的应用
IIDNameItemWork& CSyntaxItemWork::getIIDNameInf(void)
{
    if (nullptr == m_IDNameRef)
        m_IDNameRef = m_context.takeServiceReference(c_IDNameWorkSrvGUID);
    return dynamic_cast<IServiceReferenceEx&>(*m_IDNameRef).getServiceSafe<IIDNameItemWork>();
}

// 获取页面工厂的应用
IWebPageDataFactory& CSyntaxItemWork::getPageFactoryRef(void)
{
    if (nullptr == m_PageFactoryRef)
        m_PageFactoryRef = m_context.takeServiceReference(c_WebPageDataSrvGUID);
    return dynamic_cast<IServiceReferenceEx&>(*m_PageFactoryRef).getServiceSafe<IWebPageDataFactory>();
}
