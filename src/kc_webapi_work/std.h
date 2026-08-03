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
#include <vector>
#include <iostream>
#include <fstream>
#include <csignal>
#include <csetjmp>
#include <memory>
#include <atomic>
#include <random>
#include <algorithm>

#include <boost/dll.hpp>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/uuid/detail/md5.hpp>

#include "cJSON/src/cJSON.h"
#include "yyjson/src/yyjson.h"

#include "common/base_type.h"
#include "kc_web/kc_request_respond.h"
#include "util/auto_release.h"
#include "util/util_funcs_ex.h"
#include "util/ctrl_script.h"
#include "util/proc_meminfo.h"
#include "util/backtrace_symbols.h"
#include "util/bundle_activator.h"
#include "util/ctrl_sql.h"
// #include "util/load_library.h"
#include "for_cpp/kc_json_interface.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/kc_webapi_work_i.h"
#include "kc_controller/kc_parse_script_i.h"
#include "kc_controller/kc_session_cookie_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"

using namespace std;
using namespace boost;
using namespace KC;
