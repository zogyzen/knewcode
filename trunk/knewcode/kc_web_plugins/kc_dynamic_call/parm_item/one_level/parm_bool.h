#pragma once

#include "../parm_item_if.h"

namespace KC { namespace parm_item { namespace one_level
{
    // 布尔型参数
    class TParmBool : public IParmItem
    {
    public:
        // 构造
        TParmBool(boost::any&, TKcParmFL&, IFuncCallDyn::TParmRefBackPtr);

        // 添加参数
        virtual void AddArg(CKCFuncDyncall&);
        // 引用参数回调
        virtual void ParmRefBack(void);

    private:
        IFuncCallDyn::TParmRefBackPtr m_refBack;
        boost::any m_val;
        TKcParmFL m_dt;
        bool m_value = false;
    };
}}}
