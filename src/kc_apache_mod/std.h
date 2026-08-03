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

#include <httpd.h>
#include <http_core.h>
#include <http_config.h>
#include <http_protocol.h>
#include <http_connection.h>
#include <http_request.h>
#include <ap_config.h>
#include <ap_expr.h>
#include <apr_network_io.h>
#include <mod_ssl.h>

#include "common/base_type.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"
#include "kc_web/kc_websrv_proxy_i.h"
#include "util/load_websrv_proxy.h"
#include "util/load_websrv_api.h"
#include "util/auto_release.h"
#include "util/util_funcs.h"
#include "util/temp_log.h"
#include "util/backtrace_sigsegv.h"

using namespace std;
using namespace boost;
using namespace KC;
