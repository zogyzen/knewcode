#pragma once

#include "../parm_item_if.h"

namespace KC { namespace parm_item { namespace one_level
{
    // 浮点型参数
    class TParmDouble : public IParmItem
    {
    public:
        // 构造
        TParmDouble(boost::any&, TKcParmFL&, IFuncCallDyn::TParmRefBackPtr);

        // 添加参数
        virtual void AddArg(CKCFuncDyncall&);
        // 引用参数回调
        virtual void ParmRefBack(void);

    private:
        IFuncCallDyn::TParmRefBackPtr m_refBack;
        boost::any m_val;
        TKcParmFL m_dt;
        double m_value = 0;
    };
}}}
