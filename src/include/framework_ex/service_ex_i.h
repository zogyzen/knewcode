#pragma once

#include <typeinfo>

#include "framework/service_i.h"
#include "framework/bundle_context_i.h"
#include "framework_ex/bundle_ex_i.h"

namespace KC
{
	class IServiceEx : public IService
	{
	public:
		// 得到服务名（类名：return typeid(*this).name();）
                const char* CALL_TYPE getName(void) const override
		{
		    return typeid(*this).name();
        }

	public:
		// 得到模块上下文
		IBundleContext& getContext(void) const
		{
		    return this->getBundle().getContext();
		}

        // 获取本地化提示信息
        const char* CALL_TYPE getHint(const char* key) const
        {
            return this->getContext().getHint(key);
        }

        // 设置服务别名
        void SetAlias(std::string a)
        {
            m_alias = a;
        }
        // 得到服务别名
        const char* CALL_TYPE getAlias(void) override
        {
            return m_alias.empty() ? getGUID() : m_alias.c_str();
        }

		// 写日志
		bool WriteLog(TLogInfo log) const
		{
            if (EFrameworkType::eftContext == log.m_LogType) log.m_LogType = EFrameworkType::eftService;
			log.m_serviceName = this->getName();
			log.m_serviceGUID = this->getGUID();
			return dynamic_cast<const IBundleEx&>(this->getBundle()).WriteLog(log);
		}
                bool WriteLogTrace(const char* info, const char* place = "", const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvTrace)
            {
                TLogInfo log(info, place, IFuncLog::lglvTrace, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
                bool WriteLogDebug(const char* info, const char* place = "", const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvDebug)
            {
                TLogInfo log(info, place, IFuncLog::lglvDebug, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
                bool WriteLogInfo(const char* info, const char* place = "", const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvInfo)
            {
                TLogInfo log(info, place, IFuncLog::lglvInfo, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
                bool WriteLogWarning(const char* info, const char* place = "", const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvWarning)
            {
                TLogInfo log(info, place, IFuncLog::lglvWarning, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
                bool WriteLogError(const char* info, const char* place = "", const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvError)
            {
                TLogInfo log(info, place, IFuncLog::lglvError, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
                bool WriteLogFatal(const char* info, const char* place = "", const char* other = "") const
        {
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvFatal)
            {
                TLogInfo log(info, place, IFuncLog::lglvFatal, other);
                return this->WriteLog(log);
            }
            return bResult;
        }

    protected:
        ~IServiceEx() override = default;

        // 服务别名
        std::string m_alias;
	};
}
