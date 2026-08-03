#pragma once
#ifdef __GNUC__
    #pragma GCC system_header
#endif

#if defined(__MINGW32__)
    #ifndef __kernel_entry
        #define __kernel_entry
    #endif
#endif

#include <string>
#include <stdexcept>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <csignal>
#include <csetjmp>

#include <boost/smart_ptr.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>

#include "util/auto_release.h"
#include "util/util_funcs.h"
#include "util/proc_meminfo.h"
#include "util/backtrace_symbols.h"
#include "util/request_respond_tmp.h"
#include "util/bundle_activator.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/kc_web_work_i.h"
#include "kc_web/kc_webapi_work_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"

using namespace std;
using namespace boost;
using namespace KC;
