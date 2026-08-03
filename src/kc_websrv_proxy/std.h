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
#include <csignal>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/process.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/dll.hpp>

#include "util/auto_release.h"
#include "util/util_funcs.h"
#include "util/ctrl_common.h"
#include "util/temp_log.h"
#include "util/backtrace_sigsegv.h"
#include "framework/framework_i.h"
#include "framework/bundle_context_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "framework_ex/bundle_context_helper.h"
#include "kc_web/kc_websrv_proxy_i.h"
#include "kc_web/kc_web_work_i.h"
#include "kc_web/kc_webapi_work_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"

using namespace std;
using namespace boost;
using namespace KC;
