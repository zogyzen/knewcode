#pragma once

#include "grammar_base.h"
#include "grammar_expr.h"

namespace KC { namespace grammar { namespace one_level
{
    // 语法结构
    class CGrammarSyntax : public TGrammarBase<void()>
    {
    public:
        // 构造函数
        CGrammarSyntax(IWebPageData&, ICurrentParsePos&, const CGrammarBase&, const CGrammarExpr&);

    private:
        // 页面数据
        IWebPageData& m_page;
        // 当前解析位置
        ICurrentParsePos& m_currPos;

        // 基础语法
        const CGrammarBase& m_base;
        // 表达式
        const CGrammarExpr& m_expr;

    private:
        /// 关键字位置的规则
        TGrammarRule<TSynPosAttr()> rKeywordPrint, rKeywordPrintSE;
        TGrammarRule<TSynPosAttr()> rKeywordExit;
        TGrammarRule<TSynPosAttr()> rKeywordExec;
        TGrammarRule<TSynPosAttr()> rKeywordIf, rKeywordElseIf, rKeywordElse, rKeywordEndIf;
        TGrammarRule<TSynPosAttr()> rKeywordWhile, rKeywordBreak, rKeywordContinue, rKeywordEndWhile;

        /// 结构控制项规则
        TGrammarRule<void()> rSyntaxSect, rSyntaxItem;
        // 输出语句
        TGrammarRule<TKcPrintSent()> rPrint;
        // 退出语句
        TGrammarRule<TKcExitSent()> rExit;
        // 执行语句
        TGrammarRule<TKcExecSent()> rExec;
        // 分支语句
        TGrammarRule<TKcIfSent()> rIf;
        TGrammarRule<TKcElseIfSent()> rElseIf;
        TGrammarRule<TKcElseSent()> rElse;
        TGrammarRule<TKcEndIfSent()> rEndIf;
        // 循环语句
        TGrammarRule<TKcWhileSent()> rWhile;
        TGrammarRule<TKcBreakSent()> rBreak;
        TGrammarRule<TKcContinueSent()> rContinue;
        TGrammarRule<TKcEndWhileSent()> rEndWhile;

    private:
        // 设置确定关键字位置的规则
        void SetRulePos(void);
        void SetRule(void);
    };
}}}
