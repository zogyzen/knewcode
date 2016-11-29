#pragma once

namespace KC { namespace grammar
{
    // 跳过注释和空白
    struct CGrammarSkip : public qi::grammar<char*, string()>
    {
        CGrammarSkip();

    private:
        qi::rule<char*, string()> skipNote, skip;
    };

    // 规则类型
    template<typename _FT = void()> using TGrammarRule = qi::rule<char*, _FT, CGrammarSkip>;

    // 语法基类
    template<typename _FT = void()> using TGrammarBase = qi::grammar<char*, _FT, CGrammarSkip>;
}}
