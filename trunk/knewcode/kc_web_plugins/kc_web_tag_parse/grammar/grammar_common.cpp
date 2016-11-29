#include "../std.h"
#include "grammar_common.h"

////////////////////////////////////////////////////////////////////////////////
// CGrammarSkip类
KC::grammar::CGrammarSkip::CGrammarSkip() : CGrammarSkip::base_type(this->skip, "CGrammarSkip")
{
    skipNote = g_SymbolNoteLeft >> *(qi::char_ - g_SymbolNoteRight) >> g_SymbolNoteRight;
    skip = iso8859_1::space | skipNote[([&](string s){ cout << endl << "[Comments]: " << endl << s << endl; })];
}
