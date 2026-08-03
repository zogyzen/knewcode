#pragma once

#include <string>

#include "common/base_type.h"
#include "framework/func_log_i.h"

namespace KC
{
	// 日志的数据
	struct TLogInfo
	{
	public:
	    //TLogInfo(void) = delete;
        TLogInfo(std::string info, std::string place, IFuncLog::TLogLevel level = IFuncLog::lglvInfo, std::string other = "")
			: m_info(info), m_place(place), m_level(level)
            , m_other(other), m_excpID(0), m_LogType(EFrameworkType::eftService), m_refID(0) {}
		TLogInfo(TException& ex, std::string info = "")
            : m_info(ex.error_place() + "  " + info + "  " + ex.error_info()), m_place(ex.CurrPosInfo()), m_lineCode(ex.LineCode()), m_level(IFuncLog::lglvError)
                        , m_other(ex.OtherInfo()), m_excpType(ex.ExceptType()), m_excpID(ex.error_id()), m_LogType(EFrameworkType::eftService), m_refID(0) {}
        TLogInfo(const TLogInfo& c)
            : m_info(c.m_info), m_place(c.m_place), m_lineCode(c.m_lineCode), m_level(c.m_level), m_other(c.m_other)
            , m_excpType(c.m_excpType), m_excpID(c.m_excpID)
            , m_LogType(c.m_LogType), m_bundleName(c.m_bundleName), m_serviceName(c.m_serviceName), m_serviceGUID(c.m_serviceGUID), m_refID(c.m_refID)
            , m_ProcessID(c.m_ProcessID), m_threadID(c.m_threadID) {}

	public:
		// 用户输入信息
		std::string m_info;			            // 日志消息
		std::string m_place;			            // 调用的位置
        int m_lineCode = 0;                 // 源代码行数
        IFuncLog::TLogLevel m_level = IFuncLog::lglvInfo;		// 日志的等级
		std::string m_other;			            // 其他标志信息

		// 异常信息
		std::string m_excpType;		            // 异常的类型
		int m_excpID = 0;			        // 异常编号

		// 固定的信息
        EFrameworkType m_LogType = EFrameworkType::eftContext;			        // 调用类型：0框架日志；1模块日志；2服务日志；3引用日志
		std::string m_bundleName;	            // 模块的名称
		std::string m_serviceName;	            // 服务的名称（类名）
		std::string m_serviceGUID;	            // 服务的GUID
		long m_refID;			            // 服务引用的编号

		// 其它信息
		unsigned int m_ProcessID = getpid();    // 进程编号
		unsigned long int m_threadID = 0;       // 线程编号
	};
}
