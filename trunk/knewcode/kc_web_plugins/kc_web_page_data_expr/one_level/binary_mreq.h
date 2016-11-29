#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // ���ڵ��ڱȽ�����
    class TBinaryMrEq : public TBinaryNode
    {
    public:
        // ����
        TBinaryMrEq(IExprTreeWork&, IWebPageData&, const TKcEqualMoreExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
