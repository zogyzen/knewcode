#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // ȡ������
    class TBinaryModu : public TBinaryNode
    {
    public:
        // ���죬����
        TBinaryModu(IExprTreeWork&, IWebPageData&, const TKcModExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
