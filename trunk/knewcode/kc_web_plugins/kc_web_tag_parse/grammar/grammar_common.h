#pragma once

namespace KC { namespace grammar
{
    // ����ע�ͺͿհ�
    struct CGrammarSkip : public qi::grammar<char*, string()>
    {
        CGrammarSkip();

    private:
        qi::rule<char*, string()> skipNote, skip;
    };

    // ��������
    template<typename _FT = void()> using TGrammarRule = qi::rule<char*, _FT, CGrammarSkip>;

    // �﷨����
    template<typename _FT = void()> using TGrammarBase = qi::grammar<char*, _FT, CGrammarSkip>;
}}
