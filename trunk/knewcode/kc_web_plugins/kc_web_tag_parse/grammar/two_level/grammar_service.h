#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"
#include "grammar_define.h"

namespace KC { namespace grammar { namespace two_level
{
    // ����
    class CGrammarService : public TGrammarBase<void()>
    {
    public:
        // ���캯��
        CGrammarService(IWebPageData&, ICurrentParsePos&,
                        const one_level::CGrammarBase&, const one_level::CGrammarExpr&, const CGrammarDefine&);

    private:
        // ҳ������
        IWebPageData& m_page;

        // �����﷨
        const one_level::CGrammarBase& m_base;
        // ���ʽ
        const one_level::CGrammarExpr& m_expr;
        // ����
        const CGrammarDefine& m_def;

    private:
        /// ���������
        TGrammarRule<void()> rFullSect;
        TGrammarRule<string()> rService;                  // ����
        TGrammarRule<void()> rFuncDef;                    // ��������
        TGrammarRule<void()> rSrvSect;

    private:
        // ���ù���
        void SetRule(void);
    };
}}}
