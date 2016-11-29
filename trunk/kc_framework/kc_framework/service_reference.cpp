#include "std.h"
#include "service_reference.h"
#include "framework.h"

//---------------------------------------------------------------------------
// TServiceReference类
CServiceReference::CServiceReference(string symbolic, long ID, IService* srv, CBundleContext& ct)
		: m_symbolic(symbolic), m_ID(ID), m_service(srv), m_Context(ct)
{
}

CServiceReference::~CServiceReference()
{
    IServiceReference* ref = this;
	m_Context.freeServiceReference(ref);
}

// 得到引用编号
long CServiceReference::getID(void) const
{
 	return m_ID;
}

// 得到特征码
const char* CServiceReference::getGUID(void) const
{
	return m_symbolic.c_str();
}

// 得到服务名（类名）
const char* CServiceReference::getName(void) const
{
	m_name = m_symbolic;
	if (!this->disable())
		m_name = m_service->getName();
	return m_name.c_str();
}

// 服务是否失效（模块卸载、服务注销等）
bool CServiceReference::disable(void) const
{
	return nullptr == m_service;
}

// 获取服务接口
IService& CServiceReference::getService(void) const
{
	if (this->disable())
		throw TFWSrvRefException(1, __FUNCTION__, string(m_Context.getHint("The_service_don_t_exist_")) + this->getName(), this->getName(), this->getGUID(), this->getID());
 	return *m_service;
}

// 获取模块接口
const IBundle& CServiceReference::getBundle(void) const
{
	return this->getService().getBundle();
}

// 写日志
bool CServiceReference::WriteLog(TLogInfo log) const
{
	try
	{
		if (0 == log.m_LogType) log.m_LogType = 3;
		if (!this->disable())
		{
			log.m_bundleName = this->getBundle().getName();
			log.m_serviceName = this->getName();
		}
		log.m_serviceGUID = this->getGUID();
		log.m_refID = this->getID();
	}
	catch(...) {}
	return m_Context.WriteLog(log);
}

// 设置服务
void CServiceReference::setService(IService* srv)
{
    if (nullptr == srv) this->WriteLogWarning((m_Context.getHint("The_service_don_t_exist_") + m_symbolic).c_str(), __FUNCTION__, m_symbolic.c_str());
	m_service = srv;
}
