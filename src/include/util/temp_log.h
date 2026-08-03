#pragma once

#include "common/knewcode_version.h"
#include "util/util_funcs.h"
#include "util/auto_release.h"
#include "util/backtrace_symbols.h"
#include "util/proc_meminfo.h"

#include <string>
#include <fstream>

#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/process.hpp>
#include <boost/date_time.hpp>

namespace KC
{
	// 临时写日志
	class CTempLog
	{
	public:
            static void Write(std::string sInfo, std::string sPos, std::string sOther = "")
            {
                WriteInDir(boost::filesystem::initial_path<boost::filesystem::path>().string(), sInfo, sPos, sOther);
            }
            static void WriteInFile(std::string sFile, std::string sInfo, std::string sPos, std::string sOther = "")
            {
                std::string sLogFile = sFile;
                try
                {
                    std::cout << "WriteInFile - " << sLogFile << std::endl;
                    std::ofstream flog(sLogFile, std::ios::out | std::ios::app);
                    CAutoRelease _auto(boost::bind(&std::ofstream::close, &flog));
                    flog << "[" << boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()) << "] "
                         << " pid=" << boost::this_process::get_id() << "  tid=" << std::this_thread::get_id() << std::endl
                         << sPos << ": " << sInfo << std::endl << sOther << std::endl
                         << " >>> " << CProcMemInfo::Get() << " <<< " << std::endl << CBacktraceSymbols::Get() << std::endl;
                }
                catch (std::exception &ex)
                {
                    std::cout << ex.what() << std::endl << sLogFile << std::endl;
                    throw;
                }
            }
            static void WriteInDir(std::string sDir, std::string sInfo, std::string sPos, std::string sOther = "")
            {
                if (sDir.empty()) sDir = boost::filesystem::initial_path<boost::filesystem::path>().string();
                std::string sLogDir = sDir + "/logs/tmp/" + boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()).substr(0, 6) + "/";
                if (!boost::filesystem::exists(sLogDir))
                    boost::filesystem::create_directories(sLogDir);
                std::string sLogFile = sLogDir + boost::lexical_cast<std::string>(boost::this_process::get_id()) + "-" + boost::lexical_cast<std::string>(std::this_thread::get_id()) + ".log";
                try
                {
                    // std::cout << "WriteInDir - " << sLogDir << std::endl;
                    if (!boost::filesystem::exists(sLogDir)) boost::filesystem::create_directories(sLogDir);
                    CTempLog::WriteInFile(sLogFile, sInfo, sPos, sOther);
                }
                catch (std::exception &ex)
                {
                    std::cout << ex.what() << std::endl << sLogFile << std::endl;
                    throw;
                }
            }
            static void WriteInDirByDate(std::string sDir, std::string sInfo, std::string sPos, std::string sOther = "")
            {
                CTempLog::WriteInSubDirByDate(sDir, CUtilFunc::KcVersionForPathName(), sInfo, sPos, sOther);
            }
            static void WriteInSubDirByDate(std::string sDir, std::string sName, std::string sInfo, std::string sPos, std::string sOther = "")
            {
                boost::posix_time::ptime dtNow = boost::posix_time::second_clock::local_time();
                std::string sLogDir = sDir + "/logs/" + sName + "/" + boost::posix_time::to_iso_string(dtNow).substr(0, 4) + "/";
                std::string sLogFile = sLogDir + boost::posix_time::to_iso_string(dtNow).substr(0, 8) + ".log";
                try
                {
                    // std::cout << "WriteInSubDirByDate - " << sLogDir << std::endl;
                    if (!boost::filesystem::exists(sLogDir)) boost::filesystem::create_directories(sLogDir);
                    CTempLog::WriteInFile(sLogFile, sInfo, sPos, sOther);
                }
                catch (std::exception &ex)
                {
                    std::cout << ex.what() << std::endl << sLogFile << std::endl;
                    throw;
                }
            }
        };
}
