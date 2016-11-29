#include "../std.h"
#include "operand_const.h"

////////////////////////////////////////////////////////////////////////////////
// TConstNode类
KC::one_level::TConstNode::TConstNode(IExprTreeWork& wk, IWebPageData& pd, const TKcConstSyn& syn)
    : TOperandNode(wk, pd, syn), m_syn(syn)
{
}

// 获取操作数的值
boost::any KC::one_level::TConstNode::GetValue(IKCActionData&)
{
    switch (m_syn.GetVal<0>())
    {
    case EKcDtDouble:
        return m_syn.GetVal<1>();
    case EKcDtInt:
        return m_syn.GetVal<2>();
    case EKcDtBool:
        return m_syn.GetVal<3>().GetVal<0>();
    case EKcDtStr:
        return m_syn.GetVal<4>().GetVal<0>();
    default:
        return 0;
    }
}
