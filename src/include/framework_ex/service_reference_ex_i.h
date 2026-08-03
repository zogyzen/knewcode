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
        bool CALL_TYPE WriteLogTrace(const char* info, const char* place, const char* other = "") const override
        {
            TLogInfo log(info, place, IFuncLog::lglvTrace, other);
            return this->WriteLog(log);
        }
        bool CALL_TYPE WriteLogDebug(const char* info, const char* place, const char* other = "") const override
        {
            TLogInfo log(info, place, IFuncLog::lglvDebug, other);
            return this->WriteLog(log);
        }
                bool CALL_TYPE WriteLogInfo(const char* info, const char* place, const char* other = "") const override
        {
            TLogInfo log(info, place, IFuncLog::lglvInfo, other);
            return this->WriteLog(log);
        }
		virtual bool CALL_TYPE WriteLogWarning(const char* info, const char* place, const char* other = "") const
        {
            TLogInfo log(info, place, IFuncLog::lglvWarning, other);
            return this->WriteLog(log);
        }
                bool CALL_TYPE WriteLogError(const char* info, const char* place, const char* other = "") const override
        {
            TLogInfo log(info, place, IFuncLog::lglvError, other);
            return this->WriteLog(log);
        }

    public:
        // 写日志
		virtual bool CALL_TYPE WriteLog(TLogInfo) const = 0;

    public:
        template<typename ISrv>
        ISrv& getServiceSafe(void) const
        {
            if (NULL == this)
                throw TFWSrvRefException(1, __CURR_CODE_PLACE_C__, "Can't get service", std::string(this->getName()) + "(" + typeid(ISrv).name() + ")", this->getGUID(), this->getID(), "");
            ISrv* srv = dynamic_cast<ISrv*>(&this->getService());
            if (NULL == srv)
                throw TFWSrvRefException(2, __CURR_CODE_PLACE_C__, "Can't get service.", std::string(this->getName()) + "(" + typeid(ISrv).name() + ")", this->getGUID(), this->getID(), "");
            return *srv;
        }

    protected:
        ~IServiceReferenceEx() override = default;
    };
}
