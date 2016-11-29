#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // ���ڱȽ�����
    class TBinaryEqul : public TBinaryNode
    {
    public:
        // ����
        TBinaryEqul(IExprTreeWork&, IWebPageData&, const TKcEqualToExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
