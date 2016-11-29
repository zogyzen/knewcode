#include "../../std.h"
#include "grammar_define.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarDefine类
KC::grammar::two_level::CGrammarDefine::CGrammarDefine(IWebPageData& page, ICurrentParsePos& currPos,
               const one_level::CGrammarBase& base,
               const one_level::CGrammarExpr& expr
              )
        : CGrammarDefine::base_type(this->rDefSect, "CGrammarDefine")
        , m_page(page), m_currPos(currPos), m_base(base), m_expr(expr)
{
    this->SetRulePos();
    this->SetRule();
}

// 设置确定关键字位置的规则
void KC::grammar::two_level::CGrammarDefine::SetRulePos(void)
{
    // 延迟关键字
    rKeywordDelay = qi::string(g_KeywordDelay);
    // 事件关键字
    rKeywordEvent = qi::string(g_KeywordEvent);
    // 事件前关键字
    rKeywordBefore = qi::string(g_KeywordBefore);
    // 事件后关键字
    rKeywordAfter = qi::string(g_KeywordAfter);
}

// 设置规则
void KC::grammar::two_level::CGrammarDefine::SetRule(void)
{
    // 变量定义；例子，#int $m = 10, $i;
    rVarAssBody = m_expr.rVariable          [phoenix::bind(&TKcVarAssDef::SetVal<0>, _val, _1)]
            >> m_base.rEvaluateSpilit       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_expr                       [phoenix::bind(&TKcVarAssDef::SetVal<1>, _val, _1)]
            ;
    rVarAssBodyList = rVarAssBody           [phoenix::bind(&TKcVarAssListDef::PushVal, _val, _1)]
            % m_base.rCommaSpilit           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rVarDef = m_base.rCommDataType          [phoenix::bind(&TKcVarDef::SetVal<0>, _val, _1)]
            >> -(m_base.rOptionPrivate      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcVarDef::SetVal<2>, _val, true)]
               ^ m_base.rOptionReference    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcVarDef::SetVal<3>, _val, true)]
               ^ m_base.rOptionStatic       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcVarDef::SetVal<4>, _val, true)]
            ) >> rVarAssBodyList            [phoenix::bind(&TKcVarDef::SetVal<1>, _val, _1)]
            >> m_base.rSentEnd              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 变量赋值；例子，$i = &m;
    rVarAssign = rVarAssBody                [_val = _1]
            >> m_base.rSentEnd              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 内部变量赋值；例子，$$cookie["abc"] = &m;
    rInnerVarAss = m_expr.rInnerVar         [phoenix::bind(&TKcInnerVarAssDef::SetVal<0>, _val, _1)]
            >> m_base.rEvaluateSpilit       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_expr                       [phoenix::bind(&TKcInnerVarAssDef::SetVal<1>, _val, _1)]
            >> m_base.rSentEnd              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 表达式运算；例子，@mod1.ShowInfo(1);
    rExprWork = m_expr                      [phoenix::bind(&TKcExprWorkDef::SetVal<0>, _val, _1)]
            >> m_base.rSentEnd              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 类型名称（尖括号内）
    rTypeName = m_base.rSymbolLess          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_base.rCommName             [phoenix::bind(&TKcTypeNameDef::SetVal<0>, _val, _1)]
            >> m_base.rSymbolMore           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 延迟
    rDelayDef = m_base.rKeyBegin            [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordDelay                [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rTypeName                    [phoenix::bind(&TKcDelayDef::SetVal<0>, _val, _1)]
            >> (rVarAssign                  [phoenix::bind(&TKcDelayDef::SetVal<1>, _val, 0)]
                                            [phoenix::bind(&TKcDelayDef::SetVal<2>, _val, _1)]
            | rInnerVarAss                  [phoenix::bind(&TKcDelayDef::SetVal<1>, _val, 1)]
                                            [phoenix::bind(&TKcDelayDef::SetVal<3>, _val, _1)]
            | rExprWork                     [phoenix::bind(&TKcDelayDef::SetVal<1>, _val, 2)]
                                            [phoenix::bind(&TKcDelayDef::SetVal<4>, _val, _1)]
            );
    // 事件
    rEventOpPlace = m_base.rSymbolSubt      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> (rKeywordBefore              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcEventOpPlaceDef::SetVal<0>, _val, OpPlaceBefore)]
            | rKeywordAfter                 [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcEventOpPlaceDef::SetVal<0>, _val, OpPlaceAfter)]
            );
    rEventLevel = m_base.rSymbolArrayLeft   [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> qi::int_                     [phoenix::bind(&TKcEventLevelDef::SetVal<0>, _val, _1)]
            >> m_base.rSymbolArrayRight     [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rEventBody = (m_base.rAppendSpilit      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_base.rCommIDName           [phoenix::bind(&TKcEventBodyDef::SetVal<0>, _val, _1)]
            >> m_base.rCommMember           [phoenix::bind(&TKcEventBodyDef::SetVal<1>, _val, _1)]
            ) | rVarAssign                  [phoenix::bind(&TKcEventBodyDef::SetVal<2>, _val, _1)]
            | rInnerVarAss                  [phoenix::bind(&TKcEventBodyDef::SetVal<3>, _val, _1)]
            | rExprWork                     [phoenix::bind(&TKcEventBodyDef::SetVal<4>, _val, _1)]
            ;
    rEventDef = m_base.rKeyBegin            [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordEvent                [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -rEventOpPlace               [phoenix::bind(&TKcEventDef::SetVal<0>, _val, _1)]
            >> rTypeName                    [phoenix::bind(&TKcEventDef::SetVal<1>, _val, _1)]
            >> -rEventLevel                 [phoenix::bind(&TKcEventDef::SetVal<2>, _val, _1)]
            >> rEventBody                   [phoenix::bind(&TKcEventDef::SetVal<3>, _val, _1)]
            >> m_base.rSentEnd              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 配置项区
    rDefItem = rVarDef                      [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            | rDelayDef                     [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            | rEventDef                     [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            | rVarAssign                    [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            | rInnerVarAss                  [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            | rExprWork                     [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            ;
    rDefSect = m_base.rSymbHead             [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysDefID + 1)]
            >> *rDefItem
            >> m_base.rSymbTail             [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysDefID + 2)]
            ;
}
