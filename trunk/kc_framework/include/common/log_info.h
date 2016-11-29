#pragma once

#include <string>
using namespace std;

#include <boost/log/trivial.hpp>

#include "common/base_type.h"

namespace KC
{
	// 日志的数据
	struct TLogInfo
	{
	public:
		// 日志等级
		enum TLogLevel
		{
			lglvTrace = boost::log::trivial::trace,         // 调试
			lglvDebug = boost::log::trivial::debug,         // 调试
			lglvInfo = boost::log::trivial::info,			// 信息
			lglvWarning = boost::log::trivial::warning,		// 警告
			lglvError = boost::log::trivial::error,			// 错误
			lglvFatal = boost::log::trivial::fatal          // 致命
		};
	public:
	    //TLogInfo(void) = delete;
		TLogInfo(string info, string place, TLogLevel level = lglvInfo, string other = "")
			: m_info(info), m_place(place), m_level(level)
			, m_other(other), m_excpID(0), m_LogType(2), m_refID(0) {}
		TLogInfo(TException& ex, string info = "")
			: m_info(ex.CurrPosInfo() + info + ex.error_info()), m_place(ex.error_place()), m_level(lglvError)
			, m_other(ex.OtherInfo()), m_excpID(ex.error_id()), m_excpType(ex.ExceptType()), m_LogType(2), m_refID(0) {}
        TLogInfo(const TLogInfo& c)
            : m_info(c.m_info), m_place(c.m_place), m_level(c.m_level), m_other(c.m_other)
            , m_excpType(c.m_excpType), m_excpID(c.m_excpID)
            , m_LogType(c.m_LogType), m_bundleName(c.m_bundleName), m_serviceName(c.m_serviceName), m_serviceGUID(c.m_serviceGUID), m_refID(c.m_refID)
            , m_ProcessID(c.m_ProcessID), m_threadID(c.m_threadID) {}
	public:
		// 用户输入信息
		string m_info;			            // 日志消息
		string m_place;			            // 调用的位置
		TLogLevel m_level = lglvInfo;		// 日志的等级
		string m_other;			            // 其他标志信息

		// 异常信息
		string m_excpType;		            // 异常的类型
		int m_excpID = 0;			        // 异常编号

		// 固定的信息
		int m_LogType = 0;			        // 调用类型：0框架日志；1模块日志；2服务日志；3引用日志
		string m_bundleName;	            // 模块的名称
		string m_serviceName;	            // 服务的名称（类名）
		string m_serviceGUID;	            // 服务的GUID
		long m_refID;			            // 服务引用的编号

		// 其它信息
		unsigned int m_ProcessID = getpid();    // 进程编号
		unsigned long int m_threadID = 0;       // 线程编号
	};
}
