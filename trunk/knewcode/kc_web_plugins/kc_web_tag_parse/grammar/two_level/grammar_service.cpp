#include "../../std.h"
#include "grammar_service.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarService类
KC::grammar::two_level::CGrammarService::CGrammarService(IWebPageData& page, ICurrentParsePos& currPos,
               const one_level::CGrammarBase& base,
               const one_level::CGrammarExpr& expr,
               const CGrammarDefine& def
               )
        : CGrammarService::base_type(this->rFullSect, "CGrammarService")
        , m_page(page), m_base(base), m_expr(expr), m_def(def)
{
    this->SetRule();
}

// 设置规则
void KC::grammar::two_level::CGrammarService::SetRule(void)
{
    // 服务；例子，#service<name>;
    rService = qi::lit(g_SymbolKeywords) >> g_KeywordService
            >> g_SymbolLess
            >> m_base.rCommName                     [_val = _1]
            >> g_SymbolMore
            >> m_base.rSentEnd
            ;
    // 函数定义：例子、Login = $in.Login;
    rFuncDef = m_base.rCommName
            >> g_SymbolEvaluate
            >> m_base.rCommIDName
            >> m_base.rCommMember
            >> m_base.rSentEnd
            ;
    // 服务体
    rSrvSect = rService
            >> *(rFuncDef
            | m_def.rEventDef
            );
    // 整体
    rFullSect = m_base.rSymbHead
            >> rSrvSect
            >> m_base.rSymbTail;
}
