#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // ������
    class TBinarySubt : public TBinaryNode
    {
    public:
        // ����
        TBinarySubt(IExprTreeWork&, IWebPageData&, const TKcMinusExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
