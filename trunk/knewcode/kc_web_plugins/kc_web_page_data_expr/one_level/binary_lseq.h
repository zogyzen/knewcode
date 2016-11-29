#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // С�ڵ��ڱȽ�����
    class TBinaryLsEq : public TBinaryNode
    {
    public:
        // ����
        TBinaryLsEq(IExprTreeWork&, IWebPageData&, const TKcEqualLessExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
