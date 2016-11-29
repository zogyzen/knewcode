#include "std.h"
#include "activator.h"

#include "work_syntax_item.h"

// TActivator类
KC::CActivator::CActivator(const IBundle& bundle)
        : m_context(bundle.getContext()), m_bundle(bundle)
{
}

// 启动模块（注册服务等）
bool KC::CActivator::start(void)
{
	if (nullptr == m_srv)
		m_srv = new CSyntaxItemWork(m_bundle);
	if (nullptr == m_sreg)
		m_sreg = m_context.registerService(*m_srv);
	return true;
}

// 停止模块（注销服务等）
bool KC::CActivator::stop(void)
{
	m_context.unregisterService(m_sreg);
	m_sreg = nullptr;
	delete dynamic_cast<CSyntaxItemWork*>(m_srv);
	m_srv = nullptr;
	return true;
}

KC_SET_ACTIVATOR(KC::CActivator)
