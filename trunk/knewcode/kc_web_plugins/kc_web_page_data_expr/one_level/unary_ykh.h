#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // Բ��������
    class TUnaryYKH : public TUnaryNode
    {
    public:
        // ����
        TUnaryYKH(IExprTreeWork&, IWebPageData&, const TKcYKHLeftExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&);
    };
}}
