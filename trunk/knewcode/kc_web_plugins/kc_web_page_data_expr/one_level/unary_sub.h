#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // ��������
    class TUnarySub : public TUnaryNode
    {
    public:
        // ����
        TUnarySub(IExprTreeWork&, IWebPageData&, const TKcSubtExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&);
    };
}}
