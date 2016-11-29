#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // �߼�������
    class TBinaryAnd : public TBinaryNode
    {
    public:
        // ����
        TBinaryAnd(IExprTreeWork&, IWebPageData&, const TKcAndExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
