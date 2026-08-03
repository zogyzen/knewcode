#pragma once

namespace KC
{
    class CBundleContext;
    class CServiceRegistration : public IServiceRegistration
    {
    public:
        CServiceRegistration(IService& srv, CBundleContext& ct);
        ~CServiceRegistration() override;

        // 得到特征码
        const char* CALL_TYPE getGUID(void) const override;
        // 得到服务名（类名）
        const char* CALL_TYPE getName(void) const override;
        // 获取服务接口
        IService& CALL_TYPE getService(void) const override;
        // 获取模块接口
        const IBundle& CALL_TYPE getBundle(void) const override;

    private:
        // 服务接口
        IService& m_service;
        // 框架
        CBundleContext& m_Context;
    };
}
