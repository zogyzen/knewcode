#include "std.h"
#include "expr_item_if.h"

////////////////////////////////////////////////////////////////////////////////
// TOperandNode类
KC::TOperandNode::TOperandNode(IExprTreeWork& wk, IWebPageData& pd, const TKcSynBaseClass& syn)
    :  m_work(wk), m_pd(pd), m_pBeginPtr(syn.GetBeginPtr()), m_pEndPtr(syn.GetEndPtr()), m_LineID(pd.GetLineID(syn))
{
}

// 得到行号
int KC::TOperandNode::GetLineID(void) const
{
    return m_LineID;
}

////////////////////////////////////////////////////////////////////////////////
// TUnaryNode类
KC::TUnaryNode::TUnaryNode(IExprTreeWork& wk, IWebPageData& pd, const TKcSynBaseClass& syn)
    : TOperandNode(wk, pd, syn)
{
}

// 获取值
boost::any KC::TUnaryNode::GetValue(IKCActionData& act)
{
    // 操作数
    IOperandNode *pOperand = dynamic_cast<IOperandNode*>(OperandPtr.get());
    if (nullptr == pOperand)
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Not_set_operand")) % this->GetSymbol() % this->GetLineID()).str(), m_work);
    // 获取操作数的值
    boost::any val = pOperand->GetValue(act);
    // 返回计算结果
    return this->Calculate(val);
}

////////////////////////////////////////////////////////////////////////////////
// TBinaryNode类
KC::TBinaryNode::TBinaryNode(IExprTreeWork& wk, IWebPageData& pd, const TKcSynBaseClass& syn)
    : TOperandNode(wk, pd, syn)
{
}

// 获取值
boost::any KC::TBinaryNode::GetValue(IKCActionData& act)
{
    // 左操作数
    IOperandNode *pOperandL = dynamic_cast<IOperandNode*>(OperandPtrL.get());
    if (nullptr == pOperandL)
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Not_set_left_operand")) % this->GetSymbol() % this->GetLineID()).str(), m_work);
    boost::any valL = pOperandL->GetValue(act);
    // 右操作数
    IOperandNode *pOperandR = dynamic_cast<IOperandNode*>(OperandPtrR.get());
    if (nullptr == pOperandR)
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Not_set_right_operand")) % this->GetSymbol() % this->GetLineID()).str(), m_work);
    boost::any valR = pOperandR->GetValue(act);
    // 返回计算结果
    return this->Calculate(valL, valR);
}

// 得到自适应类型的左值
boost::any KC::TBinaryNode::GetLValueNewType(boost::any& lv, boost::any& rv)
{
    if (lv.empty())
    {
        if (rv.empty())
            return string("");
        else if (rv.type() == typeid(int))
            return (int)0;
        else if (rv.type() == typeid(double))
            return (double)0;
        else if (rv.type() == typeid(bool))
            return false;
        else
            return string("");
    }
    else if (rv.empty() || lv.type() == rv.type()) return lv;
    else if (lv.type() == typeid(bool))
    {
        if (rv.type() == typeid(int))
            return boost::any_cast<bool>(lv) ? (int)1 : (int)0;
        if (rv.type() == typeid(double))
            return boost::any_cast<bool>(lv) ? (double)1 : (double)0;
        else
            return boost::any_cast<bool>(lv) ? string("true") : string("false");
    }
    else if (lv.type() == typeid(int))
    {
        if (rv.type() == typeid(bool))
            return lv;
        else if (rv.type() == typeid(double))
            return (double)boost::any_cast<int>(lv);
        else
            return lexical_cast<string>(boost::any_cast<int>(lv));
    }
    else if (lv.type() == typeid(double))
    {
        if (rv.type() == typeid(bool) || rv.type() == typeid(int))
            return lv;
        else
            return lexical_cast<string>(boost::any_cast<double>(lv));
    }
    else if (lv.type() == typeid(TKcWebInfVal))
        return boost::any_cast<TKcWebInfVal>(lv).str();
    else return lv;
}
