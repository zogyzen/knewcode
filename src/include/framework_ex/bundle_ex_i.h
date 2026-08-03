#pragma once

#include "framework/bundle_i.h"
#include "common/log_info.h"

namespace KC
{
	class IBundleEx : public IBundle
	{
    public:
        // 写日志
        bool CALL_TYPE WriteLogTrace(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvTrace)
            {
                TLogInfo log(info, place, IFuncLog::lglvTrace, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
        bool CALL_TYPE WriteLogDebug(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvDebug)
            {
                TLogInfo log(info, place, IFuncLog::lglvDebug, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
                bool CALL_TYPE WriteLogInfo(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvInfo)
            {
                TLogInfo log(info, place, IFuncLog::lglvInfo, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
                virtual bool CALL_TYPE WriteLogWarning(const char* info, const char* place, const char* other = "") const
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvWarning)
            {
                TLogInfo log(info, place, IFuncLog::lglvWarning, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
                bool CALL_TYPE WriteLogError(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvError)
            {
                TLogInfo log(info, place, IFuncLog::lglvError, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
                bool CALL_TYPE WriteLogFatal(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->getContext().GetCfgLogLevel() <= (int)IFuncLog::lglvFatal)
            {
                TLogInfo log(info, place, IFuncLog::lglvFatal, other);
                return this->WriteLog(log);
            }
            return bResult;
        }

	public:
        // 写日志
		virtual bool CALL_TYPE WriteLog(TLogInfo) const = 0;

    protected:
        ~IBundleEx() override = default;
	};
}
