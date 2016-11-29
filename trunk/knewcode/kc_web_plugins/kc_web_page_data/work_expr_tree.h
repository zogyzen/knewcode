#pragma once

#include "syntax_item/syntax_item_if.h"

namespace KC
{
    // 表达式树管理接口
    class CExprTreeWork : public IExprTreeWork
    {
    public:
        CExprTreeWork(IWebPageData&);
        virtual ~CExprTreeWork() = default;

        /// 添加运算符节点的操作数
        // 添加操作数
        virtual void OperatorOperand(TKcWebExpr&, TKcWebExpr&);

        /// 添加一元运算类型节点
        // 圆括号运算
        virtual void UnaryYKH(TKcWebExpr&);
        // 负号运算
        virtual void UnarySub(TKcWebExpr&);
        // 逻辑非运算
        virtual void UnaryNot(TKcWebExpr&);

        /// 添加二元运算类型节点
        // 乘运算
        virtual void BinaryMult(TKcWebExpr&);
        // 除运算
        virtual void BinaryDivi(TKcWebExpr&);
        // 取余运算
        virtual void BinaryModu(TKcWebExpr&);
        // 加运算
        virtual void BinaryPlus(TKcWebExpr&);
        // 减运算
        virtual void BinarySubt(TKcWebExpr&);
        // 等于比较运算
        virtual void BinaryEqul(TKcWebExpr&);
        // 不等于比较运算
        virtual void BinaryNtEq(TKcWebExpr&);
        // 大于比较运算
        virtual void BinaryMore(TKcWebExpr&);
        // 小于比较运算
        virtual void BinaryLess(TKcWebExpr&);
        // 大于等于比较运算
        virtual void BinaryMrEq(TKcWebExpr&);
        // 小于等于比较运算
        virtual void BinaryLsEq(TKcWebExpr&);
        // 逻辑与运算
        virtual void BinaryAnd(TKcWebExpr&);
        // 逻辑或运算
        virtual void BinaryOr(TKcWebExpr&);

        /// 添加操作数
        // 常量
        virtual void SetConst(TKcWebExpr&, boost::any);
        // 函数调用
        virtual void SetFuncall(TKcWebExpr&, TKcWebFuncCall&);
        // 内部变量
        virtual void SetInnerVar(TKcWebExpr&, TKcWebInnerVar&);
        // 变量
        virtual void SetVar(TKcWebExpr&, TKcWebVar&);

    private:
        IWebPageData& m_WebPageData;
    };
}
