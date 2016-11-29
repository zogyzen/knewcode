#include "std.h"
#include "activator.h"

#include "parse_work.h"

// TActivator��
KC::CActivator::CActivator(const IBundle& bundle)
        : m_context(bundle.getContext()), m_bundle(bundle)
{
}

// ����ģ�飨ע�����ȣ�
bool KC::CActivator::start(void)
{
	if (nullptr == m_srv)
		m_srv = new CParseWork(m_bundle);
	if (nullptr == m_sreg)
		m_sreg = m_context.registerService(*m_srv);
	return true;
}

// ֹͣģ�飨ע������ȣ�
bool KC::CActivator::stop(void)
{
	m_context.unregisterService(m_sreg);
	m_sreg = nullptr;
	delete dynamic_cast<CParseWork*>(m_srv);
	m_srv = nullptr;
	return true;
}

KC_SET_ACTIVATOR(KC::CActivator)
