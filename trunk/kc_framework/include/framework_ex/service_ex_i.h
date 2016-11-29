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
		virtual const char* CALL_TYPE getName(void) const
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

		// 写日志
		bool WriteLog(TLogInfo log) const
		{
			if (0 == log.m_LogType) log.m_LogType = 2;
			log.m_serviceName = this->getName();
			log.m_serviceGUID = this->getGUID();
			return dynamic_cast<const IBundleEx&>(this->getBundle()).WriteLog(log);
		}
		bool WriteLogTrace(const char* info, const char* place, const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)boost::log::trivial::trace)
            {
                TLogInfo log(info, place, TLogInfo::lglvTrace, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
		bool WriteLogDebug(const char* info, const char* place, const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)boost::log::trivial::debug)
            {
                TLogInfo log(info, place, TLogInfo::lglvDebug, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
		bool WriteLogInfo(const char* info, const char* place, const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)boost::log::trivial::info)
            {
                TLogInfo log(info, place, TLogInfo::lglvInfo, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
		bool WriteLogWarning(const char* info, const char* place, const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)boost::log::trivial::warning)
            {
                TLogInfo log(info, place, TLogInfo::lglvWarning, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
		bool WriteLogError(const char* info, const char* place, const char* other = "") const
		{
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)boost::log::trivial::error)
            {
                TLogInfo log(info, place, TLogInfo::lglvError, other);
                return this->WriteLog(log);
            }
            return bResult;
		}
		bool WriteLogFatal(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getBundle().getContext().GetCfgLogLevel() <= (int)boost::log::trivial::fatal)
            {
                TLogInfo log(info, place, TLogInfo::lglvFatal, other);
                return this->WriteLog(log);
            }
            return bResult;
        }

    protected:
        virtual CALL_TYPE ~IServiceEx() = default;
	};
}
