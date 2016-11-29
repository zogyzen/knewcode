#pragma once

namespace KC
{
    class CBundleContext;
    class CServiceRegistration : public IServiceRegistration
    {
    public:
        CServiceRegistration(IService& srv, CBundleContext& ct);
        virtual CALL_TYPE ~CServiceRegistration();

        // 得到特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 得到服务名（类名）
        virtual const char* CALL_TYPE getName(void) const;
        // 获取服务接口
        virtual IService& CALL_TYPE getService(void) const;
        // 获取模块接口
        virtual const IBundle& CALL_TYPE getBundle(void) const;

    private:
        // 服务接口
        IService& m_service;
        // 框架
        CBundleContext& m_Context;
    };
}
