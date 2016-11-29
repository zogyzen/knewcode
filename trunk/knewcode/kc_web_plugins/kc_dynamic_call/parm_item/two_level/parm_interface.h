#pragma once

#include "../parm_item_if.h"

namespace KC { namespace parm_item { namespace two_level
{
    // 布尔型参数
    class TParmInterfacer : public IParmItem
    {
    public:
        // 构造
        TParmInterfacer(boost::any&, TKcParmFL&, IFuncCallDyn::TParmRefBackPtr);

        // 添加参数
        virtual void AddArg(CKCFuncDyncall&);
        // 引用参数回调
        virtual void ParmRefBack(void);

        // 转换返回值
        static boost::any TranResult(void*, string);

    private:
        IFuncCallDyn::TParmRefBackPtr m_refBack;
        boost::any m_val;
        TKcParmFL m_dt;
        TKcWebInfVal m_value;
    };
}}}
