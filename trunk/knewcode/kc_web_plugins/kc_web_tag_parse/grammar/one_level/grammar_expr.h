#pragma once

#include "grammar_base.h"

namespace KC { namespace grammar { namespace one_level
{
    // ���ʽ
    class CGrammarExpr : public TGrammarBase<TKcWebExpr()>
    {
    public:
        /// ���ù���
        TGrammarRule<TKcVarSyn()> rVariable;          // ����
        TGrammarRule<TKcInnerVarSyn()> rInnerVar;     // �ڲ�����

        // ���캯��
        CGrammarExpr(IWebPageData&, ICurrentParsePos&, const CGrammarBase&);

    private:
        // ҳ������
        IWebPageData& m_page;
        // ��ǰ����λ��
        ICurrentParsePos& m_currPos;

        // �����﷨
        const CGrammarBase& m_base;

    private:
        /// ���������
        TGrammarRule<TKcYKHLeftExpr()> rOprYKHLeft;                     // ������
        TGrammarRule<TKcYKHRightExpr()> rOprYKHRight;                   // ������
        TGrammarRule<TKcSubtExpr()> rOprSubt;                           // ����
        TGrammarRule<TKcNotExpr()> rOprNot;                             // ��
        TGrammarRule<TKcStarExpr()> rOprStar;                           // ��
        TGrammarRule<TKcDivExpr()> rOprDiv;                             // ��
        TGrammarRule<TKcModExpr()> rOprMod;                             // ȡ��
        TGrammarRule<TKcPlusExpr()> rOprPlus;                           // ��
        TGrammarRule<TKcMinusExpr()> rOprMinus;                         // ��
        TGrammarRule<TKcEqualToExpr()> rOprEqualTo;                     // ����
        TGrammarRule<TKcNotEqualExpr()> rOprNotEqual;                   // ������
        TGrammarRule<TKcEqualMoreExpr()> rOprEqualMore;                 // ���ڵ���
        TGrammarRule<TKcEqualLessExpr()> rOprEqualLess;                 // С�ڵ���
        TGrammarRule<TKcMoreExpr()> rOprMore;                           // ����
        TGrammarRule<TKcLessExpr()> rOprLess;                           // С��
        TGrammarRule<TKcAndExpr()> rOprAnd;                             // ��
        TGrammarRule<TKcOrExpr()> rOprOr;                               // ��
        TGrammarRule<TKcXorExpr()> rOprXor;                             // ���

        /// ˽�й���
        TGrammarRule<TKcExprListSyn()> rExprList;                     // ���ʽ�б�
        TGrammarRule<TKcFuncCallBodySyn()> rFuncCallBody;             // ����������
        TGrammarRule<TKcFuncCallBodyListSyn()> rFuncCallBodyList;     // ����������
        TGrammarRule<TKcFuncCallSyn()> rFuncCall;                     // ��������

        /// ���ʽ����
        TGrammarRule<TKcWebExpr()> rOperandExt;                       // ���������������á��ڲ�������
        TGrammarRule<TKcWebExpr()> rOperand;                          // ������
        TGrammarRule<TKcWebExpr()> rOperandYKH, rOperandUnary;        // ��ʱ���ϲ�����
        TGrammarRule<TKcWebExpr()> rLogic;                            // �߼�����
        TGrammarRule<TKcWebExpr()> rCompare;                          // �Ƚ�����
        TGrammarRule<TKcWebExpr()> rAddSub;                           // �Ӽ�����
        TGrammarRule<TKcWebExpr()> rMulDiv;                           // �˳�����
        TGrammarRule<TKcWebExpr()> rUnary;                            // һԪ����
        TGrammarRule<TKcWebExpr()> rYKH;                              // Բ��������
        TGrammarRule<TKcWebExpr()> rExpression;                       // ���ʽ

    private:
        // �����﷨����
        void SetRule(void);

        // ����ȷ�����������
        void SetRuleOpr(void);

        // ���ñ��ʽ����
        void SetRuleExpr(void);
    };
}}}
