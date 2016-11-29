#pragma once

#include "one_level/grammar_base.h"
#include "one_level/grammar_expr.h"
#include "one_level/grammar_syntax.h"
#include "one_level/grammar_note.h"
#include "two_level/grammar_define.h"
#include "two_level/grammar_funclib.h"
#include "two_level/grammar_service.h"
#include "two_level/grammar_include.h"

namespace KC { namespace grammar
{
    // ���﷨
    class CGrammarMain : public TGrammarBase<void()>
    {
    public:
        // ���캯��
        CGrammarMain(IWebPageData&, ICurrentParsePos&);

    private:
        // ҳ������
        IWebPageData& m_page;

    private:
        typedef CGrammarSkip _skip_type;

        // �����﷨
        const one_level::CGrammarBase m_base;
        const one_level::CGrammarExpr m_expr;
        const one_level::CGrammarNote m_note;
        // �����﷨
        const one_level::CGrammarSyntax m_syntax;
        const two_level::CGrammarDefine m_Define;
        const two_level::CGrammarFuncLib m_funcLib;
        const two_level::CGrammarService m_service;
        const two_level::CGrammarInclude m_include;

    private:
        /// ������
        TGrammarRule<> rMain;
        TGrammarRule<> rKC;                 // KC�����
        TGrammarRule<string()> rHtmlStr;    // Html�ı����ַ���
        TGrammarRule<TKcHtmlSyn()> rHtml;   // Html�ı���

    private:
        // �趨����
        void SetRule(void);
    };
}}
