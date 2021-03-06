#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 等于比较运算
    class TBinaryEqul : public TBinaryNode
    {
    public:
        // 构造
        TBinaryEqul(IExprTreeWork&, IWebPageData&, const TKcEqualToExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
