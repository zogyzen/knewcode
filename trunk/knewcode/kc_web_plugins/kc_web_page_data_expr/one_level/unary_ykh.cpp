#include "../std.h"
#include "unary_ykh.h"

////////////////////////////////////////////////////////////////////////////////
// TUnaryYKH类
KC::one_level::TUnaryYKH::TUnaryYKH(IExprTreeWork& wk, IWebPageData& pd, const TKcYKHLeftExpr& syn)
    : TUnaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TUnaryYKH::GetSymbol(void)
{
    return string() + Keychar::one_level::g_SymbolYKHLeft + Keychar::one_level::g_SymbolYKHRight;
}

// 计算
boost::any KC::one_level::TUnaryYKH::Calculate(boost::any& val)
{
    return val;
}
