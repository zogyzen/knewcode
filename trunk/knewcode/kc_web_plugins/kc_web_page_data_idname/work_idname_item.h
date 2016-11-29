#pragma once

namespace KC
{
    // 命名管理接口
    class CIDNameItemWork : public IIDNameItemWork
    {
    public:
        CIDNameItemWork(const IBundle& bundle);
        virtual CALL_TYPE ~CIDNameItemWork() = default;

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 添加变量项
        virtual TIDNameItermPtr NewVarItem(IKCActionData&, EKcDataType, string, const TKcWebExpr&);
        // 添加函数组项
        virtual TIDNameItermPtr NewLibItem(IKCActionData&, string, const TKcLoadFullFL&);

    private:
        IBundleContext& m_context;
        const IBundle& m_bundle;
    };
}
