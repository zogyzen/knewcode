#include "std.h"
#include "service_reference.h"
#include "framework.h"

//---------------------------------------------------------------------------
// TServiceReference类
CServiceReference::CServiceReference(string symbolic, unsigned long long ID, CBundleContext& ct)
        : m_symbolic(symbolic), m_ID(ID), m_Context(ct)
{
}

CServiceReference::~CServiceReference()
{
}

// 得到引用编号
unsigned long long CServiceReference::getID(void) const
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
        m_name = m_srvWPtr.lock()->getName();
	return m_name.c_str();
}

// 服务是否失效（模块卸载、服务注销等）
bool CServiceReference::disable(void) const
{
    try
    {
        if (m_srvWPtr.empty())
            m_srvWPtr = m_Context.getSrvReg(m_symbolic);
    }
    catch (TFWException &ex)
    {
        this->WriteLogTrace(ex.error_info().c_str(), __CURR_CODE_PLACE_C__, m_symbolic.c_str());
    }
    return m_srvWPtr.empty() || m_srvWPtr.expired() || m_srvWPtr.lock() == nullptr;
}

// 获取服务接口
IService& CServiceReference::getService(void) const
{
    if (this->disable())
		throw TFWSrvRefException(1, __CURR_CODE_PLACE_C__, string(m_Context.getHint("The_service_don_t_exist_")) + this->getName(), this->getName(), this->getGUID(), this->getID());
    return m_srvWPtr.lock()->getService();
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
		if (EFrameworkType::eftContext == log.m_LogType) log.m_LogType = EFrameworkType::eftReference;
        if (!m_srvWPtr.empty() && !m_srvWPtr.expired() && m_srvWPtr.lock() != nullptr)
		{
			log.m_bundleName = this->getBundle().getName();
			log.m_serviceName = this->getName();
		}
		log.m_serviceGUID = this->getGUID();
        log.m_refID = static_cast<long>(this->getID());
	}
	catch(...) {}
	return m_Context.WriteLog(log);
}
