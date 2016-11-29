#pragma once

#include "framework/service_i.h"

namespace KC
{
	// 服务引用接口
	class IServiceReference
	{
	public:
		// 得到引用编号
		virtual long CALL_TYPE getID(void) const = 0;
		// 得到服务特征码
		virtual const char* CALL_TYPE getGUID(void) const = 0;
		// 得到服务名（类名）
		virtual const char* CALL_TYPE getName(void) const = 0;
		// 服务是否已失效（模块卸载、服务注销等）
		virtual bool CALL_TYPE disable(void) const = 0;
		// 获取服务接口
		virtual IService& CALL_TYPE getService(void) const = 0;
		// 获取模块接口
		virtual const IBundle& CALL_TYPE getBundle(void) const = 0;
		// 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char* info, const char* place, const char* other = "") const = 0;
        virtual bool CALL_TYPE WriteLogDebug(const char* info, const char* place, const char* other = "") const = 0;
		virtual bool CALL_TYPE WriteLogInfo(const char* info, const char* place, const char* other = "") const = 0;
		virtual bool CALL_TYPE WriteLogWarning(const char* info, const char* place, const char* other = "") const = 0;
		virtual bool CALL_TYPE WriteLogError(const char* info, const char* place, const char* other = "") const = 0;

    protected:
        virtual CALL_TYPE ~IServiceReference() = default;
    };
}
