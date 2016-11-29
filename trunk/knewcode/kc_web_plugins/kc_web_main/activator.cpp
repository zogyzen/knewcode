#include "std.h"
#include "activator.h"

#include "kc_web_work.h"

////////////////////////////////////////////////////////////////////////////////
// 全局变量
IKCWebWork *g_WebWork = nullptr;

////////////////////////////////////////////////////////////////////////////////
// TActivator类
KC::CActivator::CActivator(const IBundle& bundle)
        : m_context(bundle.getContext()), m_bundle(bundle)
{
}

// 启动模块（注册服务等）
bool KC::CActivator::start(void)
{
	if (nullptr == m_srv)
		m_srv = new CKCWebWork(m_bundle);
	if (nullptr == m_sreg)
		m_sreg = m_context.registerService(*m_srv);
    g_WebWork = m_srv;
	return true;
}

// 停止模块（注销服务等）
bool KC::CActivator::stop(void)
{
    g_WebWork = nullptr;
	m_context.unregisterService(m_sreg);
	m_sreg = nullptr;
	delete dynamic_cast<CKCWebWork*>(m_srv);
	m_srv = nullptr;
	return true;
}

KC_SET_ACTIVATOR(KC::CActivator)
