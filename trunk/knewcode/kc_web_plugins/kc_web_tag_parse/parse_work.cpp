#include "std.h"
#include "parse_work.h"
#include "current_parse_pos.h"

#include "grammar/grammar_main.h"
using namespace grammar;

////////////////////////////////////////////////////////////////////////////////
// CParseWork类
CParseWork::CParseWork(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle)
{
}

CParseWork::~CParseWork()
{
}

// 得到服务特征码
const char* CParseWork::getGUID(void) const
{
    return c_ParseWorkSrvGUID;
}

// 对应的模块
const IBundle& CParseWork::getBundle(void) const
{
    return m_bundle;
}

bool CParseWork::Parse(IWebPageData& page)
{
    bool bResult = true;
    // 取文件信息
    const TKcWebParseTextBuffer& bufPageFile = page.GetTextBuffer();
    this->WriteLogDebug((this->getHint("Parse_page_") + bufPageFile.fileName).c_str(), __FUNCTION__,
                        lexical_cast<string>(reinterpret_cast<unsigned int>(&page)).c_str());
    // 解析
    CCurrentParsePos currPos(page);
    bResult = qi::phrase_parse(bufPageFile.step, bufPageFile.last, CGrammarMain(page, currPos), CGrammarSkip());
    bResult = bufPageFile.last == bufPageFile.step;
    if (!bResult)
    {
        const TSynPosAttr& PosAttr = currPos.Get();
        auto iLine = std::count((const char*)bufPageFile.first, PosAttr.pEnd, '\n') + 1;
        string sInfo(PosAttr.pBegin, PosAttr.pEnd + std::min(bufPageFile.last - PosAttr.pEnd, 50));
        sInfo = "(" + sInfo + "...)";
        boost::format fmtInfo = boost::format(this->getHint("Syntax_errors")) % iLine % currPos.GetSynID() % sInfo;
        TParseWorkException ex(iLine, __FUNCTION__, fmtInfo.str(), *this);
        ex.CurrPosInfo() = bufPageFile.fileName + ": line " + lexical_cast<string>(iLine) + ". \r\n";
        ex.ExceptType() = typeid(ex).name();
        throw ex;
    }
    return bResult;
}
