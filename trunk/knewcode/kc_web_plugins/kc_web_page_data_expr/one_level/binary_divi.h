#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 除运算
    class TBinaryDivi : public TBinaryNode
    {
    public:
        // 构造
        TBinaryDivi(IExprTreeWork&, IWebPageData&, const TKcDivExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
