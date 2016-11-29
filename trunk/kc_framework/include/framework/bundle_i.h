#pragma once

#include "for_user/common_define.h"

namespace KC
{
    class IBundleContext;
	class IBundle
	{
	public:
		// 模块状态
		enum TBundleState
		{
			bsFree = 0,  		// 未加载
			bsLoaded,     		// 已加载，未启动
			bsStarted			// 已启动
		};

	public:
		// 得到模块名称（子目录）
		virtual const char* CALL_TYPE getName(void) const = 0;
		// 得到模块目前状态
		virtual TBundleState CALL_TYPE getState(void) const = 0;
		// 得到所在目录
		virtual const char* CALL_TYPE getPath(void) const = 0;
		// 加载DLL
		virtual bool CALL_TYPE load(void) const = 0;
		// 释放DLL
		virtual bool CALL_TYPE free(void) const = 0;
		// 启动模块
		virtual bool CALL_TYPE start(void) const = 0;
		// 停止模块
		virtual bool CALL_TYPE stop(void) const = 0;
		// 得到模块上下文
		virtual IBundleContext& CALL_TYPE getContext(void) const = 0;
		// 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char* info, const char* place, const char* other = "") const = 0;
        virtual bool CALL_TYPE WriteLogDebug(const char* info, const char* place, const char* other = "") const = 0;
        virtual bool CALL_TYPE WriteLogInfo(const char* info, const char* place, const char* other = "") const = 0;
        virtual bool CALL_TYPE WriteLogError(const char* info, const char* place, const char* other = "") const = 0;
        virtual bool CALL_TYPE WriteLogFatal(const char* info, const char* place, const char* other = "") const = 0;

    protected:
        virtual CALL_TYPE ~IBundle() = default;
	};
}
