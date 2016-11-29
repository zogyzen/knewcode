#include "../../std.h"
#include "grammar_include.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarInclude类
KC::grammar::two_level::CGrammarInclude::CGrammarInclude(IWebPageData& page, ICurrentParsePos& currPos,
               const one_level::CGrammarBase& base,
               const one_level::CGrammarExpr& expr
               )
        : CGrammarInclude::base_type(this->rSect, "CGrammarInclude")
        , m_page(page), m_currPos(currPos), m_base(base), m_expr(expr)
{
    this->SetRulePos();
    this->SetRule();
}

// 设置确定关键字位置的规则
void KC::grammar::two_level::CGrammarInclude::SetRulePos(void)
{
    // 接口关键字
    rKeywordInclude = qi::string(g_KeywordInclude);
}

// 设置规则
void KC::grammar::two_level::CGrammarInclude::SetRule(void)
{
    // 页面引用；例子，#include "/pages/config.kc";
    rInclude = m_base.rKeyBegin         [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordInclude          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -(m_base.rOptionPrivate  [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcIncludeCL::SetVal<1>, _val, true)]
            ) >> m_expr                 [phoenix::bind(&TKcIncludeCL::SetVal<0>, _val, _1)]
            >> m_base.rSentEnd          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 整体
    rSect = m_base.rSymbHead            [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynIncludeID + 1)]
            >> +(rInclude               [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            ) >> m_base.rSymbTail       [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynIncludeID + 2)]
            ;
}
