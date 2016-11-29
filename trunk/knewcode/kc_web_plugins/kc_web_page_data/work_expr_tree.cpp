#include "std.h"
#include "work_expr_tree.h"

#include "expr_item/one_level/binary_and.h"
#include "expr_item/one_level/binary_divi.h"
#include "expr_item/one_level/binary_equl.h"
#include "expr_item/one_level/binary_less.h"
#include "expr_item/one_level/binary_lseq.h"
#include "expr_item/one_level/binary_modu.h"
#include "expr_item/one_level/binary_more.h"
#include "expr_item/one_level/binary_mreq.h"
#include "expr_item/one_level/binary_mult.h"
#include "expr_item/one_level/binary_nteq.h"
#include "expr_item/one_level/binary_or.h"
#include "expr_item/one_level/binary_plus.h"
#include "expr_item/one_level/binary_subt.h"
#include "expr_item/one_level/unary_not.h"
#include "expr_item/one_level/unary_sub.h"
#include "expr_item/one_level/unary_ykh.h"
#include "expr_item/one_level/operand_const.h"
#include "expr_item/one_level/operand_var.h"
#include "expr_item/one_level/operand_invar.h"
#include "expr_item/one_level/operand_func.h"
using namespace expr_item;
using namespace expr_item::one_level;

// 创建操作数节点
template<typename OPERAND, typename TVALUE>
OPERAND& CreateOperandNode(TKcWebExpr& exp, IWebPageData& pd, const TVALUE& val)
{
    OPERAND *pExprNode = new OPERAND(pd, val);
    TKcWebExpr::TExprTreeNodePtr NodePtr(pExprNode);
    exp.exprTreeNodePtr = NodePtr;
    return *pExprNode;
}

// 创建一元运算符节点
template<typename NODE, typename OP>
NODE& CreateUnaryOperator(TKcWebExpr& exp, IWebPageData& pd)
{
    typename NODE::TOperatorPtr OptorPtr(new OP(pd));
    NODE& ExprNode = CreateOperandNode<NODE>(exp, pd, OptorPtr);
    cout << "  " << OptorPtr->GetSymbol() << "  ";
    return ExprNode;
}

// 创建二元运算符节点
template<typename NODE, typename OP>
void CreateBinaryOperator(TKcWebExpr& exp, IWebPageData& pd)
{
    TKcWebExpr::TExprTreeNodePtr OperandPtrL = exp.exprTreeNodePtr;
    NODE& ExprNode = CreateUnaryOperator<NODE, OP>(exp, pd);
    ExprNode.OperandPtrL = OperandPtrL;
}

////////////////////////////////////////////////////////////////////////////////
// CExprTreeWork类
CExprTreeWork::CExprTreeWork(IWebPageData& pd) : m_WebPageData(pd)
{
}

/// 添加运算符节点的操作数
// 添加操作数
void CExprTreeWork::OperatorOperand(TKcWebExpr& exp, TKcWebExpr& opr)
{
    if (opr.exprTreeNodePtr.get() == nullptr)
        throw TWebPageDataFactoryException(0, __FUNCTION__, string("Operand node cannot be empty, in") + __FUNCTION__, m_WebPageData.GetFactory());
    if (exp.exprTreeNodePtr.get() == nullptr)
        throw TWebPageDataFactoryException(0, __FUNCTION__, string("Operator node cannot be empty, in ") + __FUNCTION__, m_WebPageData.GetFactory());
    // 操作符节点
    TUnaryNode* pUnary = nullptr;
    TBinaryNode* pBinary = nullptr;
    // 添加一元操作数
    if ((pUnary = dynamic_cast<TUnaryNode*>(exp.exprTreeNodePtr.get())) != nullptr)
        pUnary->OperandPtr = opr.exprTreeNodePtr;
    // 添加二元右操作数
    else if ((pBinary = dynamic_cast<TBinaryNode*>(exp.exprTreeNodePtr.get())) != nullptr)
        pBinary->OperandPtrR = opr.exprTreeNodePtr;
}

/// 添加一元运算类型节点
// 圆括号运算
void CExprTreeWork::UnaryYKH(TKcWebExpr& exp)
{
    CreateUnaryOperator<TUnaryNode, TUnaryYKH>(exp, m_WebPageData);
}

// 负号运算
void CExprTreeWork::UnarySub(TKcWebExpr& exp)
{
    CreateUnaryOperator<TUnaryNode, TUnarySub>(exp, m_WebPageData);
}

// 逻辑非运算
void CExprTreeWork::UnaryNot(TKcWebExpr& exp)
{
    CreateUnaryOperator<TUnaryNode, TUnaryNot>(exp, m_WebPageData);
}

/// 添加二元运算类型节点
// 乘运算
void CExprTreeWork::BinaryMult(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryMult>(exp, m_WebPageData);
}

// 除运算
void CExprTreeWork::BinaryDivi(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryDivi>(exp, m_WebPageData);
}

// 取余运算
void CExprTreeWork::BinaryModu(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryModu>(exp, m_WebPageData);
}

// 加运算
void CExprTreeWork::BinaryPlus(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryPlus>(exp, m_WebPageData);
}

// 减运算
void CExprTreeWork::BinarySubt(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinarySubt>(exp, m_WebPageData);
}

// 等于比较运算
void CExprTreeWork::BinaryEqul(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryEqul>(exp, m_WebPageData);
}

// 不等于比较运算
void CExprTreeWork::BinaryNtEq(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryNtEq>(exp, m_WebPageData);
}

// 大于比较运算
void CExprTreeWork::BinaryMore(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryMore>(exp, m_WebPageData);
}

// 小于比较运算
void CExprTreeWork::BinaryLess(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryLess>(exp, m_WebPageData);
}

// 大于等于比较运算
void CExprTreeWork::BinaryMrEq(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryMrEq>(exp, m_WebPageData);
}

// 小于等于比较运算
void CExprTreeWork::BinaryLsEq(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryLsEq>(exp, m_WebPageData);
}

// 逻辑与运算
void CExprTreeWork::BinaryAnd(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryAnd>(exp, m_WebPageData);
}

// 逻辑或运算
void CExprTreeWork::BinaryOr(TKcWebExpr& exp)
{
    CreateBinaryOperator<TBinaryNode, TBinaryOr>(exp, m_WebPageData);
}

/// 添加操作数
// 常量
void CExprTreeWork::SetConst(TKcWebExpr& exp, boost::any val)
{
    CreateOperandNode<TConstNode>(exp, m_WebPageData, val);
    if (val.type() == typeid(int))
        cout << boost::any_cast<int>(val);
    if (val.type() == typeid(double))
        cout << boost::any_cast<double>(val);
    if (val.type() == typeid(bool))
        cout << (boost::any_cast<bool>(val) ? "true" : "false");
    if (val.type() == typeid(string))
        cout << boost::any_cast<string>(val);
}

// 函数调用
void CExprTreeWork::SetFuncall(TKcWebExpr& exp, TKcWebFuncCall& fun)
{
    CreateOperandNode<TFuncallNode>(exp, m_WebPageData, fun);
    cout << fun.FuncFullName.GrpName << "." << fun.FuncFullName.FunName;
}

// 内部变量
void CExprTreeWork::SetInnerVar(TKcWebExpr& exp, TKcWebInnerVar& var)
{
    CreateOperandNode<TInnerVarNode>(exp, m_WebPageData, var);
    cout << var.VarName;
}

// 变量
void CExprTreeWork::SetVar(TKcWebExpr& exp, TKcWebVar& var)
{
    CreateOperandNode<TVarNode>(exp, m_WebPageData, var);
    cout << var.VarName;
}
