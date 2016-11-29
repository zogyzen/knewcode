#pragma once

#include <exception>
#include <string>
using namespace std;

#include <boost/lexical_cast.hpp>

#include "common/base_type.h"
#include "framework/service_i.h"

namespace KC
{
	// 框架异常
	class TFWException : public TException
	{
	public:
		TFWException(int id, string place, string msg) : TException(id, place, msg) {}
	};

	// 框架模块异常
	class TFWBundleException : public TFWException
	{
	public:
		TFWBundleException(int id, string place, string msg, string name)
				: TFWException(id, place, msg), m_name(name) {}
		virtual string error_info(void) const { return TFWException::error_info() + "[" + m_name + "]"; }

	protected:
		const string m_name;
	};

	//  框架服务异常
	class TFWSrvException : public TFWBundleException
	{
	public:
		TFWSrvException(int id, string place, string msg, string name, string guid)
				: TFWBundleException(id, place, msg, name), m_guid(guid) {}
		TFWSrvException(int id, string place, string msg, IService& srv)
				: TFWBundleException(id, place, msg, srv.getName()), m_guid(srv.getGUID()) {}
		virtual string error_info(void) const { return TFWBundleException::error_info() + "[" + m_guid + "]"; }

	protected:
		const string m_guid;
	};

	// 框架服务注册异常
	class TFWSrvRegException : public TFWSrvException
	{
	public:
		TFWSrvRegException(int id, string place, string msg, string name, string guid)
				: TFWSrvException(id, place, msg, name, guid) {}
	};

	// 框架服务引用异常
	class TFWSrvRefException : public TFWSrvException
	{
	public:
		TFWSrvRefException(int id, string place, string msg, string name, string guid, long RefID)
				: TFWSrvException(id, place, msg, name, guid), m_RefID(RefID) {}
		virtual string error_info(void) const
		{
			return TFWSrvException::error_info() + "[" + boost::lexical_cast<string>(m_RefID) + "]";
		}

	protected:
		const long m_RefID;
	};
}

#define CATCH_EXCEPTION_TO_WRITELOG(LogFunction, OtherSymbolic) \
	catch(KC::TException& ex) \
	{ \
		TLogInfo log(ex.error_info(), __FUNCTION__, TLogInfo::lglvError, (OtherSymbolic)); \
		log.m_excpType = typeid(ex).name(); \
		log.m_excpID = ex.error_id(); \
		(LogFunction).WriteLog(log); \
	} \
	catch(std::exception& ex) \
	{ \
		TLogInfo log(ex.what(), __FUNCTION__, TLogInfo::lglvFatal, (OtherSymbolic)); \
		log.m_excpType = typeid(ex).name(); \
		(LogFunction).WriteLog(log); \
	} \
	catch(...) \
	{ \
		TLogInfo log("Unknown exception", __FUNCTION__, TLogInfo::lglvFatal, (OtherSymbolic)); \
		(LogFunction).WriteLog(log); \
	}
