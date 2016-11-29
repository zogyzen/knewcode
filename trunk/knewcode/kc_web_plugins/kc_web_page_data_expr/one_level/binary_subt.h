#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 减运算
    class TBinarySubt : public TBinaryNode
    {
    public:
        // 构造
        TBinarySubt(IExprTreeWork&, IWebPageData&, const TKcMinusExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
