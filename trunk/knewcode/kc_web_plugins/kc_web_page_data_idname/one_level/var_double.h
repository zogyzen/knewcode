#pragma once

#include "../idname_item_if.h"

namespace KC { namespace one_level
{
    // 浮点型变量
    class TVarDouble : public TVarItemBase
    {
    public:
        // 构造，拷贝
        TVarDouble(IIDNameItemWork&, IKCActionData&, string);

        // 变量赋值
        virtual void SetVarValue(boost::any);
        // 获取变量的值
        virtual boost::any GetVarValue(void);

    private:
        // 变量值
        double m_value = 0;
    };
}}
