#pragma once

#include "../idname_item_if.h"

namespace KC { namespace one_level
{
    // 动态库中的函数
    class TFuncLib : public TFuncLibItemBase
    {
    public:
        // 构造，拷贝
        TFuncLib(IIDNameItemWork&, IKCActionData&, string, const TKcLoadFullFL&);

        // 获取函数定义
        virtual TKcFuncDefFL& GetFuncDef(string, IFuncCallDyn&);

    private:
        // 加载库内容
        TKcLoadFullFL m_loadSyn;
    };
}}
