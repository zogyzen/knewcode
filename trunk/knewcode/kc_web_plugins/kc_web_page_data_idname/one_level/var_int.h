#pragma once

#include "../idname_item_if.h"

namespace KC { namespace one_level
{
    // ���ͱ���
    class TVarInt : public TVarItemBase
    {
    public:
        // ���죬����
        TVarInt(IIDNameItemWork&, IKCActionData&, string);

        // ������ֵ
        virtual void SetVarValue(boost::any);
        // ��ȡ������ֵ
        virtual boost::any GetVarValue(void);

    private:
        // ����ֵ
        int m_value = 0;
    };
}}
