#pragma once

namespace KC
{
	class CActivator : public IBundleActivator
	{
	public:
		CActivator(const IBundle&);
		virtual CALL_TYPE ~CActivator() = default;

		// 启动模块（注册服务等）
		virtual bool CALL_TYPE start(void);
		// 停止模块（注销服务等）
		virtual bool CALL_TYPE stop(void);

    private:
        // 插件上下文
        IBundleContext& m_context;
        const IBundle& m_bundle;

        // 服务层
        IKCWebWork *m_srv = nullptr;
        IServiceRegistration *m_sreg = nullptr;
	};
}
