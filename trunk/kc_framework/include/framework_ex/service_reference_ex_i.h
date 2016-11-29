#pragma once

#include "framework/service_reference_i.h"
#include "common/log_info.h"
#include "framework_ex/framework_exception.h"

namespace KC
{
	// 服务引用接口
	class IServiceReferenceEx : public IServiceReference
	{
    public:
        // 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char* info, const char* place, const char* other = "") const
        {
            TLogInfo log(info, place, TLogInfo::lglvTrace, other);
            return this->WriteLog(log);
        }
        virtual bool CALL_TYPE WriteLogDebug(const char* info, const char* place, const char* other = "") const
        {
            TLogInfo log(info, place, TLogInfo::lglvDebug, other);
            return this->WriteLog(log);
        }
		virtual bool CALL_TYPE WriteLogInfo(const char* info, const char* place, const char* other = "") const
        {
            TLogInfo log(info, place, TLogInfo::lglvInfo, other);
            return this->WriteLog(log);
        }
		virtual bool CALL_TYPE WriteLogWarning(const char* info, const char* place, const char* other = "") const
        {
            TLogInfo log(info, place, TLogInfo::lglvWarning, other);
            return this->WriteLog(log);
        }
		virtual bool CALL_TYPE WriteLogError(const char* info, const char* place, const char* other = "") const
        {
            TLogInfo log(info, place, TLogInfo::lglvError, other);
            return this->WriteLog(log);
        }

    public:
        // 写日志
		virtual bool CALL_TYPE WriteLog(TLogInfo) const = 0;

    public:
        template<typename ISrv>
        ISrv& getServiceSafe(void) const
        {
            ISrv* srv = dynamic_cast<ISrv*>(&this->getService());
            if (NULL == srv)
                throw TFWSrvRefException(0, __FUNCTION__, "Can't get service.", string(this->getName()) + "(" + typeid(ISrv).name() + ")", this->getGUID(), this->getID());
            return *srv;
        }

    protected:
        virtual CALL_TYPE ~IServiceReferenceEx() = default;
    };
}
