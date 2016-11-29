#pragma once

#include "../idname_item_if.h"

namespace KC { namespace two_level
{
    // 接口型变量
    class TVarInterface : public TVarItemBase, public TFuncLibItemBase
    {
    public:
        // 构造，拷贝
        TVarInterface(IIDNameItemWork&, IKCActionData&, string);

        // 变量赋值
        virtual void SetVarValue(boost::any);
        // 获取变量的值
        virtual boost::any GetVarValue(void);

        // 获取函数定义
        virtual TKcFuncDefFL& GetFuncDef(string, IFuncCallDyn&);

    private:
        // 获取继承的系列接口
        void GetBaseToTheInf(vector<TKcInfFullFL*>&, string);

    private:
        // 变量值
        TKcWebInfVal m_value;
    };
}}
