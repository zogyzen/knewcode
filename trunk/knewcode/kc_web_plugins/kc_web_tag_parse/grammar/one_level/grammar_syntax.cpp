#include "../../std.h"
#include "grammar_syntax.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarSyntax类
KC::grammar::one_level::CGrammarSyntax::CGrammarSyntax(IWebPageData& page, ICurrentParsePos& currPos,
               const CGrammarBase& base,
               const CGrammarExpr& expr
              )
        : CGrammarSyntax::base_type(this->rSyntaxSect, "CGrammarSyntax")
        , m_page(page), m_currPos(currPos), m_base(base), m_expr(expr)
{
    this->SetRulePos();
    this->SetRule();
}

// 设置确定关键字位置的规则
void KC::grammar::one_level::CGrammarSyntax::SetRulePos(void)
{
    // 输出语句关键字
    rKeywordPrint = qi::string(g_KeywordPrint);
    rKeywordPrintSE = qi::string(string() + g_KeywordPrintSE);
    // 退出语句关键字
    rKeywordExit = qi::string(g_KeywordExit);
    // 执行语句关键字
    rKeywordExec = qi::string(g_KeywordExecute);
    // 分支语句关键字
    rKeywordIf = qi::string(g_KeywordIf);
    rKeywordElseIf = qi::string(g_KeywordElseIf);
    rKeywordElse = qi::string(g_KeywordElse);
    rKeywordEndIf = qi::string(g_KeywordEndIf);
    // 循环语句关键字
    rKeywordWhile = qi::string(g_KeywordWhile);
    rKeywordBreak = qi::string(g_KeywordBreak);
    rKeywordContinue = qi::string(g_KeywordContinue);
    rKeywordEndWhile = qi::string(g_KeywordEndWhile);
}

void KC::grammar::one_level::CGrammarSyntax::SetRule(void)
{
    // 输出语句；例子，#print 1 == 2, $mod1.ShowInfo(1);
    rPrint = m_base.rKeyBegin       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> (rKeywordPrint           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        | rKeywordPrintSE           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        ) >> m_expr                 [phoenix::bind(&TKcPrintSent::SetVal<0>, _val, _1)]
        >> -(m_base.rCommaSpilit    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> m_expr                   [phoenix::bind(&TKcPrintSent::SetVal<1>, _val, _1)]
        ) >> m_base.rSentEnd        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        ;
    // 退出语句；例子，#exit $mod1.ShowInfo(1), "404";
    rExit = m_base.rKeyBegin        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> rKeywordExit             [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> -(m_expr                 [phoenix::bind(&TKcExitSent::SetVal<0>, _val, _1)]
        >> -(m_base.rCommaSpilit    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> m_expr                   [phoenix::bind(&TKcExitSent::SetVal<1>, _val, _1)]
        )) >> m_base.rSentEnd       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        ;
    // 执行语句；例子，#exec 1 == 2, "<% #p 10; %>";
    rExec = m_base.rKeyBegin        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> rKeywordExec             [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> m_expr                   [phoenix::bind(&TKcExecSent::SetVal<0>, _val, _1)]
        >> -(m_base.rCommaSpilit    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        >> m_expr                   [phoenix::bind(&TKcExecSent::SetVal<1>, _val, _1)]
        ) >> m_base.rSentEnd        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
        ;
    // 条件语句：例子、#if 1 == 2、#else-if 1 == 2、#else、#end-if
    rIf = m_base.rKeyBegin          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordIf           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_expr               [phoenix::bind(&TKcIfSent::SetVal<0>, _val, _1)]
            >> m_base.rSentEnd      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rElseIf = m_base.rKeyBegin      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordElseIf       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_expr               [phoenix::bind(&TKcElseIfSent::SetVal<0>, _val, _1)]
            >> m_base.rSentEnd      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rElse = m_base.rKeyBegin        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordElse         [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_base.rSentEnd      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rEndIf = m_base.rKeyBegin       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordEndIf        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_base.rSentEnd      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 循环语句：例子、#while 1 == 2、#end-while、#break、#continue
    rWhile = m_base.rKeyBegin       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordWhile        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -(m_expr             [phoenix::bind(&TKcWhileSent::SetVal<0>, _val, _1)]
            ) >> m_base.rSentEnd    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rBreak = m_base.rKeyBegin       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordBreak        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -(m_expr             [phoenix::bind(&TKcBreakSent::SetVal<0>, _val, _1)]
            ) >> m_base.rSentEnd    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rContinue = m_base.rKeyBegin    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordContinue     [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -(m_expr             [phoenix::bind(&TKcContinueSent::SetVal<0>, _val, _1)]
            ) >> m_base.rSentEnd    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rEndWhile = m_base.rKeyBegin    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordEndWhile     [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -(m_expr             [phoenix::bind(&TKcEndWhileSent::SetVal<0>, _val, _1)]
            ) >> m_base.rSentEnd    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 结构控制
    rSyntaxItem = rPrint            [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rExit             [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rExec             [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rIf               [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rElseIf           [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rElse             [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rEndIf            [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rWhile            [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rBreak            [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rContinue         [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                | rEndWhile         [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
                ;
    rSyntaxSect = m_base.rSymbHead  [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysSentID + 1)]
                >> rSyntaxItem
                >> m_base.rSymbTail [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysSentID + 2)]
                ;
}
