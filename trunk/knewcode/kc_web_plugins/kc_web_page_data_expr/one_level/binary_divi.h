#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // ������
    class TBinaryDivi : public TBinaryNode
    {
    public:
        // ����
        TBinaryDivi(IExprTreeWork&, IWebPageData&, const TKcDivExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
