#pragma once

#include "grammar_base.h"
#include "grammar_expr.h"

namespace KC { namespace grammar { namespace one_level
{
    // �﷨�ṹ
    class CGrammarSyntax : public TGrammarBase<void()>
    {
    public:
        // ���캯��
        CGrammarSyntax(IWebPageData&, ICurrentParsePos&, const CGrammarBase&, const CGrammarExpr&);

    private:
        // ҳ������
        IWebPageData& m_page;
        // ��ǰ����λ��
        ICurrentParsePos& m_currPos;

        // �����﷨
        const CGrammarBase& m_base;
        // ���ʽ
        const CGrammarExpr& m_expr;

    private:
        /// �ؼ���λ�õĹ���
        TGrammarRule<TSynPosAttr()> rKeywordPrint, rKeywordPrintSE;
        TGrammarRule<TSynPosAttr()> rKeywordExit;
        TGrammarRule<TSynPosAttr()> rKeywordExec;
        TGrammarRule<TSynPosAttr()> rKeywordIf, rKeywordElseIf, rKeywordElse, rKeywordEndIf;
        TGrammarRule<TSynPosAttr()> rKeywordWhile, rKeywordBreak, rKeywordContinue, rKeywordEndWhile;

        /// �ṹ���������
        TGrammarRule<void()> rSyntaxSect, rSyntaxItem;
        // ������
        TGrammarRule<TKcPrintSent()> rPrint;
        // �˳����
        TGrammarRule<TKcExitSent()> rExit;
        // ִ�����
        TGrammarRule<TKcExecSent()> rExec;
        // ��֧���
        TGrammarRule<TKcIfSent()> rIf;
        TGrammarRule<TKcElseIfSent()> rElseIf;
        TGrammarRule<TKcElseSent()> rElse;
        TGrammarRule<TKcEndIfSent()> rEndIf;
        // ѭ�����
        TGrammarRule<TKcWhileSent()> rWhile;
        TGrammarRule<TKcBreakSent()> rBreak;
        TGrammarRule<TKcContinueSent()> rContinue;
        TGrammarRule<TKcEndWhileSent()> rEndWhile;

    private:
        // ����ȷ���ؼ���λ�õĹ���
        void SetRulePos(void);
        void SetRule(void);
    };
}}}
