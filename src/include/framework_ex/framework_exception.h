#pragma once

#include <exception>
#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "common/base_type.h"
#include "framework/service_i.h"
#include "util/util_funcs.h"

namespace KC
{
	// 框架异常
	class TFWException : public TException
	{
	public:
        TFWException(int id, std::string place, std::string msg, std::string name = "", std::string oth = "") : TException(id, place, msg, name, oth) {}
	};

	// 框架模块异常
	class TFWBundleException : public TFWException
	{
	public:
        TFWBundleException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                : TFWException(id, place, msg, name, oth) {}
	};

	//  框架服务异常
	class TFWSrvException : public TFWBundleException
	{
	public:
        TFWSrvException(int id, std::string place, std::string msg, std::string name, std::string guid, std::string oth)
                : TFWBundleException(id, place, msg, name, oth), m_guid(guid) {}
        TFWSrvException(int id, std::string place, std::string msg, IService& srv, std::string oth)
                : TFWBundleException(id, place, msg, srv.getName(), oth), m_guid(srv.getGUID()) {}
		virtual std::string error_info(void) const { return TFWBundleException::error_info() + "[" + m_guid + "]"; }

	protected:
		const std::string m_guid;
	};

	// 框架服务注册异常
	class TFWSrvRegException : public TFWSrvException
	{
	public:
        TFWSrvRegException(int id, std::string place, std::string msg, std::string name, std::string guid, std::string oth = "")
                : TFWSrvException(id, place, msg, name, guid, oth) {}
	};

	// 框架服务引用异常
	class TFWSrvRefException : public TFWSrvException
	{
	public:
        TFWSrvRefException(int id, std::string place, std::string msg, std::string name, std::string guid, unsigned long long RefID, std::string oth = "")
                : TFWSrvException(id, place, msg, name, guid, oth), m_RefID(RefID) {}
		virtual std::string error_info(void) const
		{
			return TFWSrvException::error_info() + "[" + boost::lexical_cast<std::string>(m_RefID) + "]";
		}

	protected:
        const unsigned long long m_RefID;
	};
}

#define CATCH_EXCEPTION_TO_WRITELOG(LogFunction, OtherSymbolic) \
	catch(KC::TException& ex) \
	{ \
        TLogInfo log(ex.error_info(), __CURR_CODE_PLACE_C__, IFuncLog::lglvError, (OtherSymbolic)); \
		log.m_excpType = typeid(ex).name(); \
		log.m_excpID = ex.error_id(); \
		(LogFunction).WriteLog(log); \
                std::cout << CUtilFunc::Utf8ToGbk(ex.error_info()) << std::endl << (OtherSymbolic) << std::endl; \
	} \
	catch(std::exception& ex) \
	{ \
        TLogInfo log(ex.what(), __CURR_CODE_PLACE_C__, IFuncLog::lglvFatal, (OtherSymbolic)); \
		log.m_excpType = typeid(ex).name(); \
		(LogFunction).WriteLog(log); \
                std::cout << CUtilFunc::Utf8ToGbk(ex.what()) << std::endl << (OtherSymbolic) << std::endl; \
        } \
	catch(...) \
	{ \
        TLogInfo log("Unknown exception", __CURR_CODE_PLACE_C__, IFuncLog::lglvFatal, (OtherSymbolic)); \
		(LogFunction).WriteLog(log); \
                std::cout << "Unknown exception" << std::endl << (OtherSymbolic) << std::endl; \
        }
