#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"

namespace KC { namespace grammar { namespace two_level
{
    // ����
    class CGrammarDefine : public TGrammarBase<void()>
    {
    public:
        // ���캯��
        CGrammarDefine(IWebPageData&, ICurrentParsePos&,
                       const one_level::CGrammarBase&, const one_level::CGrammarExpr&);

    private:
        // ҳ������
        IWebPageData& m_page;
        // ��ǰ����λ��
        ICurrentParsePos& m_currPos;

        // �����﷨
        const one_level::CGrammarBase& m_base;
        // ���ʽ
        const one_level::CGrammarExpr& m_expr;

    public:
        /// ��������
        TGrammarRule<TKcEventDef()> rEventDef;                // �¼�����
        TGrammarRule<TKcVarAssDef()> rVarAssBody;             // ������ֵ����
        TGrammarRule<TKcTypeNameDef()> rTypeName;             // �������ƣ��������ڣ�

    private:
        /// �ؼ���λ�õĹ���
        TGrammarRule<TSynPosAttr()> rKeywordDelay;            // �ӳٹؼ���
        TGrammarRule<TSynPosAttr()> rKeywordEvent;            // �¼��ؼ���
        TGrammarRule<TSynPosAttr()> rKeywordBefore;           // �¼�ǰ�ؼ���
        TGrammarRule<TSynPosAttr()> rKeywordAfter;            // �¼���ؼ���

        /// ���������
        TGrammarRule<void()> rDefSect, rDefItem;
        TGrammarRule<TKcVarAssListDef()> rVarAssBodyList;     // ������ֵ�б�
        TGrammarRule<TKcVarDef()> rVarDef;                    // ������������
        TGrammarRule<TKcVarAssDef()> rVarAssign;              // ������ֵ
        TGrammarRule<TKcInnerVarAssDef()> rInnerVarAss;       // �ڲ�������ֵ
        TGrammarRule<TKcExprWorkDef()> rExprWork;             // ���ʽ����
        TGrammarRule<TKcEventOpPlaceDef()> rEventOpPlace;     // �¼�λ��
        TGrammarRule<TKcEventLevelDef()> rEventLevel;         // �¼��ȼ�
        TGrammarRule<TKcEventBodyDef()> rEventBody;           // �¼�����
        TGrammarRule<TKcDelayDef()> rDelayDef;                // �ӳ�

    private:
        // ����ȷ���ؼ���λ�õĹ���
        void SetRulePos(void);
        // ���ù���
        void SetRule(void);
    };
}}}
