#pragma once

#include "framework/bundle_i.h"
#include "common/log_info.h"

namespace KC
{
	class IBundleEx : public IBundle
	{
    public:
        // 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)boost::log::trivial::trace)
            {
                TLogInfo log(info, place, TLogInfo::lglvTrace, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
        virtual bool CALL_TYPE WriteLogDebug(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)boost::log::trivial::debug)
            {
                TLogInfo log(info, place, TLogInfo::lglvDebug, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
		virtual bool CALL_TYPE WriteLogInfo(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)boost::log::trivial::info)
            {
                TLogInfo log(info, place, TLogInfo::lglvInfo, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
		virtual bool CALL_TYPE WriteLogWarning(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)boost::log::trivial::warning)
            {
                TLogInfo log(info, place, TLogInfo::lglvWarning, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
		virtual bool CALL_TYPE WriteLogError(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)boost::log::trivial::error)
            {
                TLogInfo log(info, place, TLogInfo::lglvError, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
		virtual bool CALL_TYPE WriteLogFatal(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)boost::log::trivial::fatal)
            {
                TLogInfo log(info, place, TLogInfo::lglvFatal, other);
                return this->WriteLog(log);
            }
            return bResult;
        }

	public:
        // 写日志
		virtual bool CALL_TYPE WriteLog(TLogInfo) const = 0;

    protected:
        virtual CALL_TYPE ~IBundleEx() = default;
	};
}
