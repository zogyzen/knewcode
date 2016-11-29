#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // С�ڱȽ�����
    class TBinaryLess : public TBinaryNode
    {
    public:
        // ����
        TBinaryLess(IExprTreeWork&, IWebPageData&, const TKcLessExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
