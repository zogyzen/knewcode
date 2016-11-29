#include "../../std.h"
#include "grammar_note.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarNote类
KC::grammar::one_level::CGrammarNote::CGrammarNote(IWebPageData& page, ICurrentParsePos& currPos,
             const CGrammarBase& base)
        : CGrammarNote::base_type(this->rNote, "CGrammarNote")
        , m_page(page), m_currPos(currPos), m_base(base)
{
    SetRule();
}

void KC::grammar::one_level::CGrammarNote::SetRule(void)
{
    /// 关键字位置的规则
    rKeywordNoteLeft = qi::string(g_KeywordNoteLeft);
    rKeywordNoteRight = qi::string(g_KeywordNoteRight);

    /// 注解项规则
    rLeft = m_base.rSymbHead        [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 1)]
            >> m_base.rKeyBegin     [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 2)]
            >> rKeywordNoteLeft     [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 3)]
            >> m_base.rSymbTail     [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 4)]
            ;
    rRight = m_base.rSymbHead       [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 5)]
            >> m_base.rKeyBegin     [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 6)]
            >> rKeywordNoteRight    [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 7)]
            >> m_base.rSymbTail     [phoenix::bind(&ICurrentParsePos::SetPos, &m_currPos, _1, c_SynSysNoteID + 8)]
            ;
    rBody = *(qi::char_ - rRight);
    // 注解；例子，<!--<% #note %>-->字段<!--<% #end-note %>-->
    rNote = rLeft >> rBody[([&](string s){ /* cout << endl << "[Note]: " << endl << s << endl; */ })] >> rRight;
}
