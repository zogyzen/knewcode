#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 取余运算
    class TBinaryModu : public TBinaryNode
    {
    public:
        // 构造，拷贝
        TBinaryModu(IExprTreeWork&, IWebPageData&, const TKcModExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
