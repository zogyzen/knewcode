#pragma once

#include <boost/spirit/include/qi.hpp>

#include "kc_web/web_struct.h"

namespace KC
{
    //////////////////////////////////////解析语法/////////////////////////////////////
    namespace grammar
    {
        // 跳过注释和空白
        struct CGrammarSkip : public boost::spirit::qi::grammar<char*, string()>
        {
            CGrammarSkip() : CGrammarSkip::base_type(this->skip, "CGrammarSkip")
            {
                skipNote = Keychar::one_level::g_SymbolNoteLeft >> *(boost::spirit::qi::char_ - Keychar::one_level::g_SymbolNoteRight) >> Keychar::one_level::g_SymbolNoteRight;
                skip = boost::spirit::iso8859_1::space | skipNote[([&](string s){ cout << endl << "[Comments]: " << endl << s << endl; })];
            }

        private:
            boost::spirit::qi::rule<char*, string()> skipNote, skip;
        };

        // 规则类型
        template<typename _FT = void()> using TGrammarRule = boost::spirit::qi::rule<char*, _FT, CGrammarSkip>;

        // 语法基类
        template<typename _FT = void()> using TGrammarBase = boost::spirit::qi::grammar<char*, _FT, CGrammarSkip>;

        // 当前解析的位置
        struct ICurrentParsePos
        {
            // 设置当前解析的位置
            virtual void Set(TSynSectInBuf&, const TSynPosAttr&) = 0;
            virtual void SetPos(const TSynPosAttr&, int) = 0;

        protected:
            virtual ~ICurrentParsePos(void) = default;
        };
    }
}
