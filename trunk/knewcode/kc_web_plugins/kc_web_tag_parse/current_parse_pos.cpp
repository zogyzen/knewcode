#include "std.h"
#include "current_parse_pos.h"

////////////////////////////////////////////////////////////////////////////////
// CCurrentParsePos类
CCurrentParsePos::CCurrentParsePos(IWebPageData& pg) : m_page(pg), m_pos(pg.GetCurrPos())
{
}

CCurrentParsePos::~CCurrentParsePos()
{
}

// 设置当前解析的位置
void CCurrentParsePos::Set(TSynSectInBuf& syn, const TSynPosAttr& pos)
{
    syn.SynPosAttrList->push_back(pos);
    this->SetPosBase(pos, syn.GetID());
}

void CCurrentParsePos::SetPos(const TSynPosAttr& pos, int synID)
{
    m_page.SetPrevPos(pos);
    this->SetPosBase(pos, synID);
}

inline void CCurrentParsePos::SetPosBase(const TSynPosAttr& pos, int synID)
{
    // cout << "  " << string(pos.pBegin, pos.pEnd) << "  ";
    m_page.SetCurrPos(pos);
    m_SynID = synID;
    m_pos = pos;
}

// 得到当前解析位置
const TSynPosAttr& CCurrentParsePos::Get(void)
{
    return m_pos;
}

int CCurrentParsePos::GetSynID(void)
{
    return m_SynID;
}
