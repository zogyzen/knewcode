#pragma once

#include "../idname_item_if.h"

namespace KC { namespace one_level
{
    // 整型变量
    class TVarInt : public TVarItemBase
    {
    public:
        // 构造，拷贝
        TVarInt(IIDNameItemWork&, IKCActionData&, string);

        // 变量赋值
        virtual void SetVarValue(boost::any);
        // 获取变量的值
        virtual boost::any GetVarValue(void);

    private:
        // 变量值
        int m_value = 0;
    };
}}
