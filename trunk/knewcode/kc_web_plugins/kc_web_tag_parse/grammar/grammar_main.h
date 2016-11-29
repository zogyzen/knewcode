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
    // 主语法
    class CGrammarMain : public TGrammarBase<void()>
    {
    public:
        // 构造函数
        CGrammarMain(IWebPageData&, ICurrentParsePos&);

    private:
        // 页面数据
        IWebPageData& m_page;

    private:
        typedef CGrammarSkip _skip_type;

        // 基本语法
        const one_level::CGrammarBase m_base;
        const one_level::CGrammarExpr m_expr;
        const one_level::CGrammarNote m_note;
        // 各项语法
        const one_level::CGrammarSyntax m_syntax;
        const two_level::CGrammarDefine m_Define;
        const two_level::CGrammarFuncLib m_funcLib;
        const two_level::CGrammarService m_service;
        const two_level::CGrammarInclude m_include;

    private:
        /// 主规则
        TGrammarRule<> rMain;
        TGrammarRule<> rKC;                 // KC标记区
        TGrammarRule<string()> rHtmlStr;    // Html文本区字符串
        TGrammarRule<TKcHtmlSyn()> rHtml;   // Html文本区

    private:
        // 设定规则
        void SetRule(void);
    };
}}
