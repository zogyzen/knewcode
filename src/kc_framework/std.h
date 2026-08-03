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
#include <vector>
#include <deque>
#include <map>
#include <stdexcept>
#include <csignal>
#include <csetjmp>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <atomic>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/dll.hpp>

#include <plusaes/plusaes.hpp>

#include "common/knewcode_version.h"
#include "common/base_type.h"
#include "util/auto_release.h"
#include "util/util_funcs_ex.h"
#include "util/proc_meminfo.h"
#include "util/system_run_status.h"
#include "util/load_library.h"
#include "util/load_sub_lib.h"
#include "util/temp_log.h"
#include "util/lock_work.h"
#include "util/backtrace_sigsegv.h"
#include "util/backtrace_symbols.h"
#include "kc_web/kc_main_work_i.h"
#include "framework/func_log_i.h"
#include "framework/framework_i.h"
#include "framework/bundle_activator_i.h"
#include "framework/service_registration_i.h"
#include "framework_ex/bundle_context_ex_i.h"
#include "framework_ex/bundle_ex_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"

using namespace std;
using namespace boost;
using namespace KC;
