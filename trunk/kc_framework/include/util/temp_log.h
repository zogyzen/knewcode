#pragma once

#include <string>
#include <fstream>
using namespace std;

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "common/knewcode_version.h"
#include "util/auto_release.h"
#include "util/get_sysinfo.h"
#include "util/backtrace_symbols.h"
#include "util/proc_meminfo.h"

namespace KC
{
	// 临时写日志
	class CTempLog
	{
	public:
	    static void Write(string sInfo, string sPos = __FUNCTION__)
	    {
            string sLogDir = CGetSysInfo::GetLogDir() + c_KnewcodeVersion;
            if (!boost::filesystem::exists(sLogDir)) boost::filesystem::create_directories(sLogDir);
            string sLogFile = sLogDir + "/" + boost::lexical_cast<string>(getpid()) + "-" + boost::lexical_cast<string>(pthread_self()) + ".log";
            ofstream flog(sLogFile, ios::out | ios::app);
            CAutoRelease _auto(boost::bind(&ofstream::close, &flog));
            flog << "[" << posix_time::to_iso_string(posix_time::second_clock::local_time()) << "] " << sInfo << endl << sPos << endl
                 << (" - >>> " + CProcMemInfo::Get() + " <<<") << endl << CBacktraceSymbols::Get() << endl;
	    }
	};
}
