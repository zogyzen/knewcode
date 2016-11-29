#pragma once

namespace KC
{
    // 变量项
    class TVarItemBase : public IVarItemBase
    {
    public:
        // 获取页面活动数据
        virtual IKCActionData& GetAction(void);

    public:
        TVarItemBase(IIDNameItemWork&, IKCActionData&, EKcDataType, string);

        // 创建变量
        static TIDNameItermPtr CreateVarItem(IIDNameItemWork&, IKCActionData&, EKcDataType, string, const TKcWebExpr&);

    protected:
        IIDNameItemWork& m_work;
        IKCActionData& m_act;
        // 类型
        EKcDataType m_dt;
        // 变量名
        string m_varName;
    };

    // 函数组项
    class TFuncLibItemBase : public IFuncLibItemBase
    {
    public:
        // 获取页面活动数据
        virtual IKCActionData& GetAction(void);

    public:
        TFuncLibItemBase(IIDNameItemWork&, IKCActionData&, string);

        // 创建函数组
        static TIDNameItermPtr CreateLibItem(IIDNameItemWork&, IKCActionData&, string, const TKcLoadFullFL&);

    protected:
        IIDNameItemWork& m_work;
        IKCActionData& m_act;
        // 库名
        string m_libName;
    };
}
