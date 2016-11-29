#pragma once

namespace KC
{
    class CInnerVar : public IInnerVar
    {
    public:
        CInnerVar(const IBundle& bundle);
        virtual CALL_TYPE ~CInnerVar();

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 是否存在
        virtual bool Exists(IActionData&, string, TAnyTypeValList&) const;
        // 得到内部变量值
        virtual boost::any GetValue(IActionData&, string, TAnyTypeValList&);
        // 设置内部变量值
        virtual void SetValue(IActionData&, string, TAnyTypeValList&, boost::any);

    private:
        // 插件及上下文
        IBundleContext& m_context;
        const IBundle& m_bundle;
    };
}
