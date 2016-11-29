#include "std.h"
#include "event_work.h"
#include "action_data.h"
#include "expr_item/expr_item_if.h"
#include "syntax_item/one_level/var_ass_item.h"
#include "syntax_item/one_level/invar_ass_item.h"
#include "syntax_item/one_level/expr_item.h"

////////////////////////////////////////////////////////////////////////////////
// CEventWork类
KC::CEventWork::CEventWork(CActionData& act) : m_act(act)
{
}

// 添加事件
void KC::CEventWork::AddEvent(TKcEventDef& ev)
{
    // 等级
    int iLv = 0;
    TOperandNode *lvExpr = dynamic_cast<TOperandNode*>(ev.levelExpr.exprTreeNodePtr.get());
    if (nullptr != lvExpr)
    {
        boost::any lvVal = lvExpr->GetValue(m_act);
        if (lvVal.type() == typeid(int)) iLv = boost::any_cast<int>(lvVal);
    }
    // 查找相同名称事件列表
    auto iterName = m_EventMap.find(ev.EventName);
    if (m_EventMap.end() == iterName)
    {
        // 插入新事件名称
        TEventLevelMap* pEvLvMap = new TEventLevelMap;
        TEventLevelMapPtr EvLvMapPtr(pEvLvMap);
        m_EventMap.insert(make_pair(ev.EventName, EvLvMapPtr));
        iterName = m_EventMap.find(ev.EventName);
    }
    // 查找相同等级的事件列表
    auto iterLevel = iterName->second->find(iLv);
    if (iterName->second->end() == iterLevel)
    {
        // 新的事件等级
        TEventVector* pEvVct = new TEventVector;
        TEventVectorPtr EvVctPtr(pEvVct);
        iterName->second->insert(make_pair(iLv, EvVctPtr));
        iterLevel = iterName->second->find(iLv);
    }
    // 插入事件
    iterLevel->second->push_back(&ev);
}

// 触发事件
void KC::CEventWork::TrigerEvent(string name, const TKcEventDef::EOpPlace op)
{
    try
    {
        // 查找事件列表
        auto iterName = m_EventMap.find(name);
        if (m_EventMap.end() != iterName)
            // 从高到低循环每个等级
            for(auto iterLevel = iterName->second->rbegin(); iterName->second->rend() != iterLevel; ++iterLevel)
                // 按后进先出的方式循环每个相同等级的事件
                for(auto iterEvent = iterLevel->second->rbegin(); iterLevel->second->rend() != iterEvent; ++iterEvent)
                    try
                    {
                        // 获取事件定义指针
                        TKcEventDef* pEvDef = *iterEvent;
                        if (nullptr == pEvDef)
                            throw "The event define pointer is NULL.";
                        // 执行事件
                        if (op == pEvDef->OperatePlace)
                        {
                            switch (pEvDef->OperateType)
                            {
                            case TKcEventDef::optVarAss:        // 变量赋值
                                syntax_item::one_level::CVarAssItem::AssignValue(pEvDef->VarAss, m_act);
                                break;
                            case TKcEventDef::optInVarAss:      // 内部变量赋值
                                syntax_item::one_level::CInnerVarAssItem::AssignValue(pEvDef->InVarAss, m_act);
                                break;
                            case TKcEventDef::optExprWork:      // 表达式运算
                                syntax_item::one_level::CExprItem::AssignExpr(pEvDef->ExprWork, m_act);
                                break;
                            }
                        }
                    }
                    catch(CKCException& ex)
                    {
                        m_act.GetWebPageData().GetFactory().WriteLogError(("Triger event [" + name + "] error. " + ex.error_info()).c_str(), __FUNCTION__, m_act.GetRequestRespond().GetLocalFilename());
                    }
                    catch(std::exception& ex)
                    {
                        m_act.GetWebPageData().GetFactory().WriteLogError(("Triger event [" + name + "] error. " + ex.what()).c_str(), __FUNCTION__, m_act.GetRequestRespond().GetLocalFilename());
                    }
                    catch(const char* str)
                    {
                        m_act.GetWebPageData().GetFactory().WriteLogError(("Triger event [" + name + "] error. " + str).c_str(), __FUNCTION__, m_act.GetRequestRespond().GetLocalFilename());
                    }
                    catch(...)
                    {
                        m_act.GetWebPageData().GetFactory().WriteLogError(("Triger event [" + name + "] error. unknown error.").c_str(), __FUNCTION__, m_act.GetRequestRespond().GetLocalFilename());
                    }
    }
    catch(std::exception& ex)
    {
        m_act.GetWebPageData().GetFactory().WriteLogError(("Triger event [" + name + "] error.. " + ex.what()).c_str(), __FUNCTION__, m_act.GetRequestRespond().GetLocalFilename());
    }
    catch(...)
    {
        m_act.GetWebPageData().GetFactory().WriteLogError(("Triger event [" + name + "] error.. unknown error.").c_str(), __FUNCTION__, m_act.GetRequestRespond().GetLocalFilename());
    }
}
