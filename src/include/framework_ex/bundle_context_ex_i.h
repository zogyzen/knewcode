#pragma once

#include "common/base_type.h"
#include "common/log_info.h"
#include "framework/bundle_context_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "framework_ex/framework_exception.h"

namespace KC
{
    class IBundleContextEx : public IBundleContext
    {
    public:
        // 写日志
        bool CALL_TYPE WriteLogTrace(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->GetCfgLogLevel() <= (int)IFuncLog::lglvTrace)
            {
                TLogInfo log(info, place, IFuncLog::lglvTrace, other);
                bResult = this->WriteLog(log);
            }
            return bResult;
        }
        bool CALL_TYPE WriteLogDebug(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->GetCfgLogLevel() <= (int)IFuncLog::lglvDebug)
            {
                TLogInfo log(info, place, IFuncLog::lglvDebug, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
        bool CALL_TYPE WriteLogInfo(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->GetCfgLogLevel() <= (int)IFuncLog::lglvInfo)
            {
                TLogInfo log(info, place, IFuncLog::lglvInfo, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
        bool CALL_TYPE WriteLogWarning(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->GetCfgLogLevel() <= (int)IFuncLog::lglvWarning)
            {
                TLogInfo log(info, place, IFuncLog::lglvWarning, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
        bool CALL_TYPE WriteLogError(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->GetCfgLogLevel() <= (int)IFuncLog::lglvError)
            {
                TLogInfo log(info, place, IFuncLog::lglvError, other);
                return this->WriteLog(log);
            }
            return bResult;
        }
        bool CALL_TYPE WriteLogFatal(const char* info, const char* place, const char* other = "") const override
        {
            bool bResult = true;
            if (this->GetCfgLogLevel() <= (int)IFuncLog::lglvFatal)
            {
                TLogInfo log(info, place, IFuncLog::lglvFatal, other);
                return this->WriteLog(log);
            }
            return bResult;
        }

    public:
        // 写日志
        virtual bool CALL_TYPE WriteLog(TLogInfo) const = 0;
        // 获取服务接口
        IServiceReferenceEx& getServiceRef(const char* GUID)
        {
            IServiceReferenceEx* srvRef = dynamic_cast<IServiceReferenceEx*>(this->takeServiceReference(GUID));
            if (nullptr == srvRef)
                throw TFWSrvRefException(0, __CURR_CODE_PLACE_C__, "Can't get service reference.", "", GUID, -1, "");
            return *srvRef;
        }
        template<typename IF>
        IF& getService(const char* GUID)
        {
            return this->getServiceRef(GUID).getServiceSafe<IF>();
        }

        // 配置
        virtual boost::property_tree::ptree& CfgPt(void) = 0;

    protected:
        ~IBundleContextEx(void) override = default;
    };
}
