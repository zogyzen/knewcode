#pragma once

namespace KC
{
    class CBundleContext;
    class CServiceReference : public IServiceReferenceEx
    {
    public:
        CServiceReference(string symbolic, unsigned long long ID, CBundleContext& ct);
        ~CServiceReference() override;

        // 得到引用编号
        unsigned long long CALL_TYPE getID(void) const override;
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override;
        // 得到服务名（类名）
        const char* CALL_TYPE getName(void) const override;
        // 服务是否已失效（模块卸载、服务注销等）
        bool CALL_TYPE disable(void) const override;
        // 获取服务接口
        IService& CALL_TYPE getService(void) const override;
        // 获取模块接口
        const IBundle& CALL_TYPE getBundle(void) const override;
        // 写日志
        bool CALL_TYPE WriteLog(TLogInfo) const override;

    private:
        // 唯一特征码
        string m_symbolic;
        // 名称
        mutable string m_name;
        // 编号
        const unsigned long long m_ID;
        // 框架
        CBundleContext& m_Context;
        // 服务的注册，弱指针
        mutable boost::weak_ptr<IServiceRegistration> m_srvWPtr;
    };
}
