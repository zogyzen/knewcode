#pragma once

#include "grammar_base.h"

namespace KC { namespace grammar { namespace one_level
{
    // 表达式
    class CGrammarExpr : public TGrammarBase<TKcWebExpr()>
    {
    public:
        /// 公用规则
        TGrammarRule<TKcVarSyn()> rVariable;          // 变量
        TGrammarRule<TKcInnerVarSyn()> rInnerVar;     // 内部变量

        // 构造函数
        CGrammarExpr(IWebPageData&, ICurrentParsePos&, const CGrammarBase&);

    private:
        // 页面数据
        IWebPageData& m_page;
        // 当前解析位置
        ICurrentParsePos& m_currPos;

        // 基础语法
        const CGrammarBase& m_base;

    private:
        /// 运算符规则
        TGrammarRule<TKcYKHLeftExpr()> rOprYKHLeft;                     // 左括号
        TGrammarRule<TKcYKHRightExpr()> rOprYKHRight;                   // 右括号
        TGrammarRule<TKcSubtExpr()> rOprSubt;                           // 负号
        TGrammarRule<TKcNotExpr()> rOprNot;                             // 非
        TGrammarRule<TKcStarExpr()> rOprStar;                           // 乘
        TGrammarRule<TKcDivExpr()> rOprDiv;                             // 除
        TGrammarRule<TKcModExpr()> rOprMod;                             // 取余
        TGrammarRule<TKcPlusExpr()> rOprPlus;                           // 加
        TGrammarRule<TKcMinusExpr()> rOprMinus;                         // 减
        TGrammarRule<TKcEqualToExpr()> rOprEqualTo;                     // 等于
        TGrammarRule<TKcNotEqualExpr()> rOprNotEqual;                   // 不等于
        TGrammarRule<TKcEqualMoreExpr()> rOprEqualMore;                 // 大于等于
        TGrammarRule<TKcEqualLessExpr()> rOprEqualLess;                 // 小于等于
        TGrammarRule<TKcMoreExpr()> rOprMore;                           // 大于
        TGrammarRule<TKcLessExpr()> rOprLess;                           // 小于
        TGrammarRule<TKcAndExpr()> rOprAnd;                             // 与
        TGrammarRule<TKcOrExpr()> rOprOr;                               // 或
        TGrammarRule<TKcXorExpr()> rOprXor;                             // 异或

        /// 私有规则
        TGrammarRule<TKcExprListSyn()> rExprList;                     // 表达式列表
        TGrammarRule<TKcFuncCallBodySyn()> rFuncCallBody;             // 函数调用体
        TGrammarRule<TKcFuncCallBodyListSyn()> rFuncCallBodyList;     // 函数调用体
        TGrammarRule<TKcFuncCallSyn()> rFuncCall;                     // 函数调用

        /// 表达式规则
        TGrammarRule<TKcWebExpr()> rOperandExt;                       // 操作数（函数调用、内部变量）
        TGrammarRule<TKcWebExpr()> rOperand;                          // 操作数
        TGrammarRule<TKcWebExpr()> rOperandYKH, rOperandUnary;        // 临时复合操作数
        TGrammarRule<TKcWebExpr()> rLogic;                            // 逻辑运算
        TGrammarRule<TKcWebExpr()> rCompare;                          // 比较运算
        TGrammarRule<TKcWebExpr()> rAddSub;                           // 加减运算
        TGrammarRule<TKcWebExpr()> rMulDiv;                           // 乘除运算
        TGrammarRule<TKcWebExpr()> rUnary;                            // 一元运算
        TGrammarRule<TKcWebExpr()> rYKH;                              // 圆括号运算
        TGrammarRule<TKcWebExpr()> rExpression;                       // 表达式

    private:
        // 设置语法规则
        void SetRule(void);

        // 设置确定运算符规则
        void SetRuleOpr(void);

        // 设置表达式规则
        void SetRuleExpr(void);
    };
}}}
