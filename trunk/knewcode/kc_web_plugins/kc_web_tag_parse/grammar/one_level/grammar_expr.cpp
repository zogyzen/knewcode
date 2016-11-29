#include "../../std.h"
#include "grammar_expr.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarExpr类
KC::grammar::one_level::CGrammarExpr::CGrammarExpr(IWebPageData& page, ICurrentParsePos& currPos,
             const CGrammarBase& base)
        : CGrammarExpr::base_type(this->rExpression, "CGrammarExpr")
        , m_page(page), m_currPos(currPos), m_base(base)
{
    this->SetRule();
    this->SetRuleOpr();
    this->SetRuleExpr();
}

// 设置语法规则
void KC::grammar::one_level::CGrammarExpr::SetRule(void)
{
    // 变量
    rVariable = m_base.rCommIDName          [_val = _1]
            ;
    // 表达式列表
    rExprList = rExpression                 [phoenix::bind(&TKcExprListSyn::PushVal, _val, _1)]
            % m_base.rCommaSpilit           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 内部变量
    rInnerVar = m_base.rIDNameInnerBegin    [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_base.rCommName             [phoenix::bind(&TKcInnerVarSyn::SetVal<0>, _val, _1)]
            >> m_base.rSymbolArrayLeft      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -rExprList                   [phoenix::bind(&TKcInnerVarSyn::SetVal<1>, _val, _1)]
            >> m_base.rSymbolArrayRight     [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 函数调用
    rFuncCallBody = m_base.rCommMember      [phoenix::bind(&TKcFuncCallBodySyn::SetVal<0>, _val, _1)]
            >> m_base.rSymbolYKHLeft        [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -rExprList                   [phoenix::bind(&TKcFuncCallBodySyn::SetVal<1>, _val, _1)]
            >> m_base.rSymbolYKHRight       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    rFuncCallBodyList = +(rFuncCallBody     [phoenix::bind(&TKcFuncCallBodyListSyn::PushVal, _val, _1)]
            );
    rFuncCall = (rOperandExt                [phoenix::bind(&TKcFuncCallSyn::SetVal<1>, _val, _1)]
            | m_base.rCommIDName            [phoenix::bind(&TKcFuncCallSyn::SetVal<0>, _val, _1)]
            ) >> rFuncCallBodyList          [phoenix::bind(&TKcFuncCallSyn::SetVal<2>, _val, _1)]
            ;
}

// 设置确定运算符规则
void KC::grammar::one_level::CGrammarExpr::SetRuleOpr(void)
{
    // 括号
    rOprYKHLeft = m_base.rSymbolYKHLeft[_val = _1];
    rOprYKHRight = m_base.rSymbolYKHRight[_val = _1];
    // 负号
    rOprSubt = m_base.rSymbolSubt[_val = _1];
    // 非
    rOprNot = qi::string(string() + g_SymbolNot);
    // 乘、除、取余
    rOprStar = qi::string(string() + g_SymbolStar);
    rOprDiv = qi::string(string() + g_SymbolDiv);
    rOprMod = qi::string(string() + g_SymbolMod);
    // 加、减
    rOprPlus = qi::string(string() + g_SymbolPlus);
    rOprMinus = m_base.rSymbolSubt[_val = _1];
    // 等于、不等于、大于等于、小于等于、大于、小于
    rOprEqualTo = qi::string(g_SymbolEqualTo);
    rOprNotEqual = qi::string(g_SymbolNotEqual);
    rOprEqualMore = qi::string(g_SymbolEqualMore);
    rOprEqualLess = qi::string(g_SymbolEqualLess);
    rOprMore = qi::string(string() + g_SymbolMore);
    rOprLess = qi::string(string() + g_SymbolLess);
    // 与、或、异或
    rOprAnd = qi::string(g_SymbolAnd);
    rOprOr = qi::string(g_SymbolOr);
    rOprXor = qi::string(string() + g_SymbolXor);
}

// 设置表达式规则
void KC::grammar::one_level::CGrammarExpr::SetRuleExpr(void)
{
    // 操作数
    rOperandExt = rInnerVar                 [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
             ;
    rOperand = m_base.rCommConst            [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
             | rFuncCall                    [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
             | rOperandExt                  [_val = _1]
             | rVariable                    [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
             ;
    // 圆括号运算
    rYKH = rOprYKHLeft                      [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            >> rExpression                  [phoenix::bind(&IWebPageData::SetExprOperand, &m_page, _val, _1)]
            >> rOprYKHRight                 [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            ;
    // 其他一元运算
    rOperandYKH = rYKH[_val = _1] | rOperand[_val = _1];
    rUnary = (rOprSubt                      [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            | rOprNot                       [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            ) >> rOperandYKH                [phoenix::bind(&IWebPageData::SetExprOperand, &m_page, _val, _1)]
            ;
    // 乘除运算
    rOperandUnary = rUnary[_val = _1] | rOperandYKH[_val = _1];
    rMulDiv = rOperandUnary                 [_val = _1]
            >> *((rOprStar                  [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprDiv                   [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprMod                   [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            ) >> rOperandUnary              [phoenix::bind(&IWebPageData::SetExprOperand, &m_page, _val, _1)]
            );
    // 加减运算
    rAddSub = rMulDiv                       [_val = _1]
            >> *((rOprPlus                  [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprMinus                 [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            ) >> rMulDiv                    [phoenix::bind(&IWebPageData::SetExprOperand, &m_page, _val, _1)]
            );
    // 比较运算
    rCompare = rAddSub                      [_val = _1]
            >> *((rOprEqualTo               [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprNotEqual              [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprEqualMore             [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprEqualLess             [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprMore                  [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprLess                  [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            ) >> rAddSub                    [phoenix::bind(&IWebPageData::SetExprOperand, &m_page, _val, _1)]
            );
    // 逻辑运算
    rLogic = rCompare                       [_val = _1]
            >> *((rOprAnd                   [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprOr                    [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
                | rOprXor                   [phoenix::bind(&IWebPageData::SetExprOperator, &m_page, _val, _1)]
            ) >> rCompare                   [phoenix::bind(&IWebPageData::SetExprOperand, &m_page, _val, _1)]
            );
    // 表达式
    rExpression = rLogic[_val = _1];
}
