#include "std.h"
#include "work_expr_tree.h"

#include "one_level/binary_and.h"
#include "one_level/binary_divi.h"
#include "one_level/binary_equl.h"
#include "one_level/binary_less.h"
#include "one_level/binary_lseq.h"
#include "one_level/binary_modu.h"
#include "one_level/binary_more.h"
#include "one_level/binary_mreq.h"
#include "one_level/binary_mult.h"
#include "one_level/binary_nteq.h"
#include "one_level/binary_or.h"
#include "one_level/binary_xor.h"
#include "one_level/binary_plus.h"
#include "one_level/binary_subt.h"
#include "one_level/unary_not.h"
#include "one_level/unary_sub.h"
#include "one_level/unary_ykh.h"
#include "one_level/operand_const.h"
#include "one_level/operand_var.h"
#include "one_level/operand_invar.h"
#include "one_level/operand_func.h"
using namespace one_level;

////////////////////////////////////////////////////////////////////////////////
// CExprTreeWork类
CExprTreeWork::CExprTreeWork(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle)
{
}

// 得到服务特征码
const char* CExprTreeWork::getGUID(void) const
{
    return c_ExprTreeWorkSrvGUID;
}

// 对应的模块
const IBundle& CExprTreeWork::getBundle(void) const
{
    return m_bundle;
}

// 创建操作数节点
template<typename OPERAND, typename SYN>
OPERAND& CExprTreeWork::CreateOperandNode(TKcWebExpr& exp, IWebPageData& pd, const SYN& syn)
{
    OPERAND *pExprNode = new OPERAND(*this, pd, syn);
    TKcWebExpr::TExprTreeNodePtr NodePtr(pExprNode);
    exp.exprTreeNodePtr = NodePtr;
    return *pExprNode;
}

// 创建一元运算符节点
template<typename OPERATOR, typename SYN>
OPERATOR& CExprTreeWork::CreateUnaryOperator(TKcWebExpr& exp, IWebPageData& pd, const SYN& syn)
{
    return CreateOperandNode<OPERATOR>(exp, pd, syn);
}

// 创建二元运算符节点
template<typename OPERATOR, typename SYN>
OPERATOR& CExprTreeWork::CreateBinaryOperator(TKcWebExpr& exp, IWebPageData& pd, const SYN& syn)
{
    TKcWebExpr::TExprTreeNodePtr OperandPtrL = exp.exprTreeNodePtr;
    OPERATOR& ExprNode = CreateUnaryOperator<OPERATOR>(exp, pd, syn);
    ExprNode.OperandPtrL = OperandPtrL;
    return ExprNode;
}

// 设置表达式项运算符（参数依次为：页数据接口、表达式项、运算符或语法）
IOperandNode* CExprTreeWork::SetExprOperator(IWebPageData& pd, TKcWebExpr& expr, const TKcSynBaseClass& syn)
{
    IOperandNode* pResult = nullptr;
    /// 一元运算符
    // 圆括号
    if (syn.GetID() == c_SynSysExprID_YKHLeft && dynamic_cast<const TKcYKHLeftExpr*>(&syn) != nullptr)
        pResult = &this->CreateUnaryOperator<TUnaryYKH>(expr, pd, dynamic_cast<const TKcYKHLeftExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_YKHRight && dynamic_cast<const TKcYKHRightExpr*>(&syn) != nullptr)
        ;
    // 负号、逻辑非
    else if (syn.GetID() == c_SynSysExprID_Subt && dynamic_cast<const TKcSubtExpr*>(&syn) != nullptr)
        pResult = &this->CreateUnaryOperator<TUnarySub>(expr, pd, dynamic_cast<const TKcSubtExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_Not && dynamic_cast<const TKcNotExpr*>(&syn) != nullptr)
        pResult = &this->CreateUnaryOperator<TUnaryNot>(expr, pd, dynamic_cast<const TKcNotExpr&>(syn));
    /// 二元运算符
    // 乘、除、取余
    else if (syn.GetID() == c_SynSysExprID_Star && dynamic_cast<const TKcStarExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryMult>(expr, pd, dynamic_cast<const TKcStarExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_Div && dynamic_cast<const TKcDivExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryDivi>(expr, pd, dynamic_cast<const TKcDivExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_Mod && dynamic_cast<const TKcModExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryModu>(expr, pd, dynamic_cast<const TKcModExpr&>(syn));
    // 加、减
    else if (syn.GetID() == c_SynSysExprID_Plus && dynamic_cast<const TKcPlusExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryPlus>(expr, pd, dynamic_cast<const TKcPlusExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_Minus && dynamic_cast<const TKcMinusExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinarySubt>(expr, pd, dynamic_cast<const TKcMinusExpr&>(syn));
    // 比较运算
    else if (syn.GetID() == c_SynSysExprID_EqualTo && dynamic_cast<const TKcEqualToExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryEqul>(expr, pd, dynamic_cast<const TKcEqualToExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_NotEqual && dynamic_cast<const TKcNotEqualExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryNtEq>(expr, pd, dynamic_cast<const TKcNotEqualExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_More && dynamic_cast<const TKcMoreExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryMore>(expr, pd, dynamic_cast<const TKcMoreExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_Less && dynamic_cast<const TKcLessExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryLess>(expr, pd, dynamic_cast<const TKcLessExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_EqualMore && dynamic_cast<const TKcEqualMoreExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryMrEq>(expr, pd, dynamic_cast<const TKcEqualMoreExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_EqualLess && dynamic_cast<const TKcEqualLessExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryLsEq>(expr, pd, dynamic_cast<const TKcEqualLessExpr&>(syn));
    // 逻辑运算
    else if (syn.GetID() == c_SynSysExprID_And && dynamic_cast<const TKcAndExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryAnd>(expr, pd, dynamic_cast<const TKcAndExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_Or && dynamic_cast<const TKcOrExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryOr>(expr, pd, dynamic_cast<const TKcOrExpr&>(syn));
    else if (syn.GetID() == c_SynSysExprID_Xor && dynamic_cast<const TKcXorExpr*>(&syn) != nullptr)
        pResult = &this->CreateBinaryOperator<TBinaryXor>(expr, pd, dynamic_cast<const TKcXorExpr&>(syn));
    /// 操作数
    // 常量
    else if (syn.GetID() == c_SynSysBaseID_Const && dynamic_cast<const TKcConstSyn*>(&syn) != nullptr)
        pResult = &this->CreateOperandNode<TConstNode>(expr, pd, dynamic_cast<const TKcConstSyn&>(syn));
    // 函数调用
    else if (syn.GetID() == c_SynSysBaseID_FuncCall && dynamic_cast<const TKcFuncCallSyn*>(&syn) != nullptr)
        pResult = &this->CreateOperandNode<TFuncallNode>(expr, pd, dynamic_cast<const TKcFuncCallSyn&>(syn));
    // 内部变量
    else if (syn.GetID() == c_SynSysBaseID_InnerVar && dynamic_cast<const TKcInnerVarSyn*>(&syn) != nullptr)
        pResult = &this->CreateOperandNode<TInnerVarNode>(expr, pd, dynamic_cast<const TKcInnerVarSyn&>(syn));
    // 变量
    else if (syn.GetID() == c_SynSysBaseID_IDName && dynamic_cast<const TKcVarSyn*>(&syn) != nullptr)
        pResult = &this->CreateOperandNode<TVarNode>(expr, pd, dynamic_cast<const TKcVarSyn&>(syn));
    return pResult;
}

// 设置表达式项操作数（参数依次为：页数据接口、运算符表达式项、操作数）
IOperandNode* CExprTreeWork::SetExprOperand(IWebPageData& pd, TKcWebExpr& opr, const TKcWebExpr& exp)
{
    if (exp.exprTreeNodePtr.get() == nullptr)
        throw TExprTreeWorkSrvException(pd.GetCurrLineID(), __FUNCTION__, this->getHint("Operand_node_cannot_be_empty_") + lexical_cast<string>(pd.GetCurrLineID()), *this);
    if (opr.exprTreeNodePtr.get() == nullptr)
        throw TExprTreeWorkSrvException(pd.GetCurrLineID(), __FUNCTION__, this->getHint("Operator_node_cannot_be_empty_") + lexical_cast<string>(pd.GetCurrLineID()), *this);
    // 操作符节点
    TUnaryNode* pUnary = nullptr;
    TBinaryNode* pBinary = nullptr;
    // 添加一元操作数
    if ((pUnary = dynamic_cast<TUnaryNode*>(opr.exprTreeNodePtr.get())) != nullptr)
        pUnary->OperandPtr = exp.exprTreeNodePtr;
    // 添加二元右操作数
    else if ((pBinary = dynamic_cast<TBinaryNode*>(opr.exprTreeNodePtr.get())) != nullptr)
        pBinary->OperandPtrR = exp.exprTreeNodePtr;
    return dynamic_cast<IOperandNode*>(opr.exprTreeNodePtr.get());
}

// 获取标识符的应用
IDynamicCall& CExprTreeWork::getDynamicCallInf(void)
{
    if (nullptr == m_DynamicCallRef)
        m_DynamicCallRef = m_context.takeServiceReference(c_DynamicCallSrvGUID);
    return dynamic_cast<IServiceReferenceEx&>(*m_DynamicCallRef).getServiceSafe<IDynamicCall>();
}

// 获取内部变量的应用
IInnerVar& CExprTreeWork::getInnerVarInf(void)
{
    if (nullptr == m_InnerVarRef)
        m_InnerVarRef = m_context.takeServiceReference(c_InnerVarSrvGUID);
    return dynamic_cast<IServiceReferenceEx&>(*m_InnerVarRef).getServiceSafe<IInnerVar>();
}

// 获取标识符的应用
IIDNameItemWork& CExprTreeWork::getIDNameInf(void)
{
    if (nullptr == m_IDNameRef)
        m_IDNameRef = m_context.takeServiceReference(c_IDNameWorkSrvGUID);
    return dynamic_cast<IServiceReferenceEx&>(*m_IDNameRef).getServiceSafe<IIDNameItemWork>();
}
