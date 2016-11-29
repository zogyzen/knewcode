#include "std.h"
#include "service_registration.h"
#include "framework.h"

//---------------------------------------------------------------------------
// TServiceRegistration类
CServiceRegistration::CServiceRegistration(IService& srv, CBundleContext& ct)
		: m_service(srv), m_Context(ct)
{
}

CServiceRegistration::~CServiceRegistration()
{
}

// 得到特征码
const char* CServiceRegistration::getGUID(void) const
{
	return m_service.getGUID();
}

// 得到服务名（类名）
const char* CServiceRegistration::getName(void) const
{
	return m_service.getName();
}

// 获取服务接口
IService& CServiceRegistration::getService(void) const
{
 	return m_service;
}

// 获取模块接口
const IBundle& CServiceRegistration::getBundle(void) const
{
	return m_service.getBundle();
}
