#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"

namespace KC { namespace grammar { namespace two_level
{
    // ����
    class CGrammarInclude : public TGrammarBase<void()>
    {
    public:
        // ���캯��
        CGrammarInclude(IWebPageData&, ICurrentParsePos&,
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

    private:
        /// �ؼ���λ�õĹ���
        TGrammarRule<TSynPosAttr()> rKeywordInclude;      // �����ؼ���

        /// ���������
        TGrammarRule<void()> rSect;
        TGrammarRule<TKcIncludeCL()> rInclude;            // ҳ������

    private:
        // ����ȷ���ؼ���λ�õĹ���
        void SetRulePos(void);
        // ���ù���
        void SetRule(void);
    };
}}}
