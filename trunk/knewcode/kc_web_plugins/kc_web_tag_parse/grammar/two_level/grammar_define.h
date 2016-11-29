#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"

namespace KC { namespace grammar { namespace two_level
{
    // 定义
    class CGrammarDefine : public TGrammarBase<void()>
    {
    public:
        // 构造函数
        CGrammarDefine(IWebPageData&, ICurrentParsePos&,
                       const one_level::CGrammarBase&, const one_level::CGrammarExpr&);

    private:
        // 页面数据
        IWebPageData& m_page;
        // 当前解析位置
        ICurrentParsePos& m_currPos;

        // 基本语法
        const one_level::CGrammarBase& m_base;
        // 表达式
        const one_level::CGrammarExpr& m_expr;

    public:
        /// 公共规则
        TGrammarRule<TKcEventDef()> rEventDef;                // 事件定义
        TGrammarRule<TKcVarAssDef()> rVarAssBody;             // 变量赋值部分
        TGrammarRule<TKcTypeNameDef()> rTypeName;             // 类型名称（尖括号内）

    private:
        /// 关键字位置的规则
        TGrammarRule<TSynPosAttr()> rKeywordDelay;            // 延迟关键字
        TGrammarRule<TSynPosAttr()> rKeywordEvent;            // 事件关键字
        TGrammarRule<TSynPosAttr()> rKeywordBefore;           // 事件前关键字
        TGrammarRule<TSynPosAttr()> rKeywordAfter;            // 事件后关键字

        /// 定义项规则
        TGrammarRule<void()> rDefSect, rDefItem;
        TGrammarRule<TKcVarAssListDef()> rVarAssBodyList;     // 变量赋值列表
        TGrammarRule<TKcVarDef()> rVarDef;                    // 变量定义整体
        TGrammarRule<TKcVarAssDef()> rVarAssign;              // 变量赋值
        TGrammarRule<TKcInnerVarAssDef()> rInnerVarAss;       // 内部变量赋值
        TGrammarRule<TKcExprWorkDef()> rExprWork;             // 表达式运算
        TGrammarRule<TKcEventOpPlaceDef()> rEventOpPlace;     // 事件位置
        TGrammarRule<TKcEventLevelDef()> rEventLevel;         // 事件等级
        TGrammarRule<TKcEventBodyDef()> rEventBody;           // 事件主体
        TGrammarRule<TKcDelayDef()> rDelayDef;                // 延迟

    private:
        // 设置确定关键字位置的规则
        void SetRulePos(void);
        // 设置规则
        void SetRule(void);
    };
}}}
