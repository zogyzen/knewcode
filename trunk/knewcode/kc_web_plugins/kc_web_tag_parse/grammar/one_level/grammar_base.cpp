#include "../../std.h"
#include "grammar_base.h"

///////////////////////////////////自定义分析器/////////////////////////////////
// 纯浮点型分析器（需小数点）
template <typename T>
struct strict_real_policies : qi::real_policies<T>
{
    static bool const expect_dot = true;
};
qi::real_parser<double, strict_real_policies<double>> ts_real_;

////////////////////////////////////////////////////////////////////////////////
// CGrammarBase类
KC::grammar::one_level::CGrammarBase::CGrammarBase(IWebPageData& page, ICurrentParsePos& currPos)
        : CGrammarBase::base_type(this->rUnuse, "CGrammarBase")
        , m_page(page), m_currPos(currPos)
{
    this->SetRulePos();
    this->SetRuleCommon();
}

// 设置确定语法位置的规则
void KC::grammar::one_level::CGrammarBase::SetRulePos(void)
{
    // 标记头、尾
    rSymbHead = -qi::no_skip[qi::lit(g_SymbolHeadUn)] >> qi::string(g_SymbolHead);
    rSymbTail = qi::string(g_SymbolTail) >> -qi::no_skip[qi::lit(g_SymbolTailUn)];
    // 关键字开始
    rKeyBegin = qi::string(string() + g_SymbolKeywords);
    // 标识符开始
    rIDNameBegin = qi::string(string() + g_SymbolIDName);
    // 内部标识符开始
    rIDNameInnerBegin = qi::string(g_SymbolIDNameInner);
    // 成员引用开始
    rMemberBegin = qi::string(string() + g_SymbolDot);
    // 语句结束
    rSentEnd = qi::string(string() + g_SymbolSemic);
    // 逗号分割
    rCommaSpilit = qi::string(string() + g_SymbolComma);
    // 冒号分割
    rCommaColon = qi::string(string() + g_SymbolColon);
    // 双引号范围
    rDbQuotesRange = qi::string(string() + g_SymbolDbQuotes);
    // 圆括号
    rSymbolYKHLeft = qi::string(string() + g_SymbolYKHLeft);
    rSymbolYKHRight = qi::string(string() + g_SymbolYKHRight);
    // 方括号
    rSymbolArrayLeft = qi::string(string() + g_SymbolArrayLeft);
    rSymbolArrayRight = qi::string(string() + g_SymbolArrayRight);
    // 赋值符号分割
    rEvaluateSpilit = qi::string(string() + g_SymbolEvaluate);
    // 减号
    rSymbolSubt = qi::string(string() + g_SymbolSubt);
    // 尖括号
    rSymbolMore = qi::string(string() + g_SymbolMore);
    rSymbolLess = qi::string(string() + g_SymbolLess);
    // 添加符号分割
    rAppendSpilit = qi::string(g_SymbolAppend);
    // 引用符号
    rSymbolRef = qi::string(string() + g_SymbolRef);
    // 私有定义选项
    rOptionPrivate = qi::string(g_OptionPrivate);
    // 引用定义选项
    rOptionReference = qi::string(g_OptionReference);
    // 静态定义选项
    rOptionStatic = qi::string(g_OptionStatic);

    /// 关键字
    rKeywordVoid = qi::string(g_KeywordVoid);
    rKeywordInterface = qi::string(g_KeywordInterface);
    rKeywordBool = qi::string(g_KeywordBool);
    rKeywordInt = qi::string(g_KeywordInt);
    rKeywordDouble = qi::string(g_KeywordDouble);
    rKeywordStr = qi::string(g_KeywordStr);
    rKeywordTrue = qi::string(g_KeywordTrue);
    rKeywordFalse = qi::string(g_KeywordFalse);
}

// 设置基本规则
void KC::grammar::one_level::CGrammarBase::SetRuleCommon(void)
{
    // 名称
    rCommName = (qi::alpha | qi::char_(g_SymbolUnderline)) >> *(qi::alnum | qi::char_(g_SymbolUnderline));
    // 字符串
    rCommStrText = qi::no_skip[*((qi::char_(g_SymbolDbQuotes) >> qi::lit(g_SymbolDbQuotes)) | ~qi::lit(g_SymbolDbQuotes))];
    rCommStr = rDbQuotesRange           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rCommStrText             [phoenix::bind(&TKcStringSyn::SetVal<0>, _val, _1)]
            >> rDbQuotesRange           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            ;
    // 标识符
    rCommIDName = rIDNameBegin          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rCommName                [phoenix::bind(&TKcIDNameSyn::SetVal<0>, _val, _1)]
            ;
    // 成员引用
    rCommMember = rMemberBegin          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> rCommName                [phoenix::bind(&TKcMemberSyn::SetVal<0>, _val, _1)]
            ;
    // 数据类型
    rCommDataType = rKeyBegin           [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> (rKeywordVoid            [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcDataTypeSyn::SetVal<0>, _val, EKcDtVoid)]
            | rKeywordInterface         [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcDataTypeSyn::SetVal<0>, _val, EKcDtInterface)]
            | rKeywordBool              [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcDataTypeSyn::SetVal<0>, _val, EKcDtBool)]
            | rKeywordInt               [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcDataTypeSyn::SetVal<0>, _val, EKcDtInt)]
            | rKeywordDouble            [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcDataTypeSyn::SetVal<0>, _val, EKcDtDouble)]
            | rKeywordStr               [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcDataTypeSyn::SetVal<0>, _val, EKcDtStr)]
            );
    // 常量
    rCommConstBool = rKeyBegin          [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
            >> (rKeywordTrue            [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcConstBoolSyn::SetVal<0>, _val, true)]
            | rKeywordFalse             [phoenix::bind(&ICurrentParsePos::Set, &m_currPos, _val, _1)]
                                        [phoenix::bind(&TKcConstBoolSyn::SetVal<0>, _val, false)]
            );
    rCommConst = ts_real_               [phoenix::bind(&TKcConstSyn::SetVal<1>, _val, _1)]
                                        [phoenix::bind(&TKcConstSyn::SetVal<0>, _val, EKcDtDouble)]
            | qi::int_                  [phoenix::bind(&TKcConstSyn::SetVal<2>, _val, _1)]
                                        [phoenix::bind(&TKcConstSyn::SetVal<0>, _val, EKcDtInt)]
            | rCommConstBool            [phoenix::bind(&TKcConstSyn::SetVal<3>, _val, _1)]
                                        [phoenix::bind(&TKcConstSyn::SetVal<0>, _val, EKcDtBool)]
            | rCommStr                  [phoenix::bind(&TKcConstSyn::SetVal<4>, _val, _1)]
                                        [phoenix::bind(&TKcConstSyn::SetVal<0>, _val, EKcDtStr)]
            ;
}
