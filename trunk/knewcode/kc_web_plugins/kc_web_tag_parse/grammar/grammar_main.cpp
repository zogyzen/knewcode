#include "../std.h"
#include "grammar_main.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarMain类
KC::grammar::CGrammarMain::CGrammarMain(IWebPageData& page, ICurrentParsePos& currPos)
            : CGrammarMain::base_type(this->rMain, "CGrammarMain"), m_page(page)
            , m_base(page, currPos), m_expr(page, currPos, m_base), m_note(page, currPos, m_base)
            , m_syntax(page, currPos, m_base, m_expr), m_Define(page, currPos, m_base, m_expr)
            , m_funcLib(page, currPos, m_base, m_expr, m_Define), m_service(page, currPos, m_base, m_expr, m_Define)
            , m_include(page, currPos, m_base, m_expr)
{
    SetRule();
};

// 设定规则
void KC::grammar::CGrammarMain::SetRule(void)
{
    // KC标记区
    rKC = m_note | m_syntax | m_funcLib | m_service | m_include | m_Define;
    // Html文本区
    rHtmlStr = +(qi::no_skip[qi::char_] - (-qi::no_skip[qi::lit(g_SymbolHeadUn)] >> qi::no_skip[qi::lit(g_SymbolHead)]));
    rHtml = rHtmlStr    [phoenix::bind(&TKcHtmlSyn::SetVal<0>, _val, _1)]
            ;
    // 主规则
    rMain = *(rKC
            | rHtml     [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            );
}
