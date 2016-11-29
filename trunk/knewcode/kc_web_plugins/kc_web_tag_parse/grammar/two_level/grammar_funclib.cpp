#include "../../std.h"
#include "grammar_funclib.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarFuncLib类
KC::grammar::two_level::CGrammarFuncLib::CGrammarFuncLib(IWebPageData& page, ICurrentParsePos& currPos,
               const one_level::CGrammarBase& base,
               const one_level::CGrammarExpr& expr,
               const CGrammarDefine& def
               )
        : CGrammarFuncLib::base_type(this->rFullSect, "CGrammarFuncLib")
        , m_page(page), m_currPos(currPos), m_base(base), m_expr(expr), m_def(def)
{
    this->SetRulePos();
    this->SetRule();
}

// 设置确定关键字位置的规则
void KC::grammar::two_level::CGrammarFuncLib::SetRulePos(void)
{
    // 接口关键字
    rKeywordInterface = qi::string(g_KeywordInterface);
    // 加载关键字
    rKeywordLoadMod = qi::string(g_KeywordLoadMod);
}

// 设置规则
void KC::grammar::two_level::CGrammarFuncLib::SetRule(void)
{
    // 形参
    rParm = m_base.rCommDataType            [phoenix::bind(&TKcParmFL::SetVal<0>, _val, _1)]
            >> -m_def.rTypeName             [phoenix::bind(&TKcParmFL::SetVal<1>, _val, _1)]
            >> -m_base.rSymbolRef           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcParmFL::SetVal<2>, _val, true)]
            ;
    // 形参列表
    rParms = m_base.rSymbolYKHLeft          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -(rParm                      [phoenix::bind(&TKcParmsFL::PushVal, _val, _1)]
            % m_base.rCommaSpilit           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ) >> m_base.rSymbolYKHRight     [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 函数声明；例子，#void ShowInfo(#int);
    rFunction = rParm                       [phoenix::bind(&TKcFuncDefFL::SetVal<0>, _val, _1)]
            >> m_base.rCommName             [phoenix::bind(&TKcFuncDefFL::SetVal<1>, _val, _1)]
            >> rParms                       [phoenix::bind(&TKcFuncDefFL::SetVal<2>, _val, _1)]
            >> m_base.rSentEnd              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 函数声明列表
    rFunctions = *rFunction                 [phoenix::bind(&TKcFuncDefFL::SetVal<3>, _1, phoenix::bind(&TKcFuncDefsFL::Count, _val))]
                                            [phoenix::bind(&TKcFuncDefsFL::PushVal, _val, _1)]
            ;
    // 接口；例子，#interface<name>;
    rInterface = m_base.rKeyBegin           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordInterface            [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_def.rTypeName              [phoenix::bind(&TKcInfFL::SetVal<0>, _val, _1)]
            >> -(m_base.rOptionPrivate      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcInfFL::SetVal<2>, _val, true)]
            ) >> -(m_base.rCommaColon       [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> m_def.rTypeName              [phoenix::bind(&TKcInfFL::SetVal<1>, _val, _1)]
            ) >> m_base.rSentEnd            [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 完整接口
    rInterfaceFull = rInterface             [phoenix::bind(&TKcInfFullFL::SetVal<0>, _val, _1)]
            >> rFunctions                   [phoenix::bind(&TKcInfFullFL::SetVal<1>, _val, _1)]
            ;
    // 模块加载；例子，#load $mod1 = "/bin/sample";
    rLoadMod = m_base.rKeyBegin             [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rKeywordLoadMod              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> -(m_base.rOptionPrivate      [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                            [phoenix::bind(&TKcLoadFL::SetVal<2>, _val, true)]
            ) >> m_def.rVarAssBody          [phoenix::bind(&TKcLoadFL::SetVal<0>, _val, _1)]
            >> m_base.rSentEnd              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 完整模块加载
    rLoadModFull = rLoadMod                 [phoenix::bind(&TKcLoadFullFL::SetVal<0>, _val, _1)]
            >> rFunctions                   [phoenix::bind(&TKcLoadFullFL::SetVal<1>, _val, _1)]
            ;
    // 模块函数区
    rFullSect = m_base.rSymbHead            [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysLibID + 1)]
            >> (rInterfaceFull              [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            | rLoadModFull                  [phoenix::bind(&IWebPageData::AddSyntaxItem, &m_page, _1)]
            ) >> m_base.rSymbTail           [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysLibID + 2)]
            ;
}
