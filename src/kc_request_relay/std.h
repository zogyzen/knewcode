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
#include <map>
#include <set>
#include <iostream>
#include <fstream>

#include <httplib.h>

#include <boost/smart_ptr.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>

#include "common/base_type.h"
#include "util/auto_release.h"
#include "util/util_http.h"
#include "util/ctrl_script.h"
#include "util/proc_meminfo.h"
#include "util/backtrace_symbols.h"
#include "util/bundle_activator.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_exception.h"
#include "kc_controller/kc_relay_rest_i.h"
#include "for_user/kc_single_loadso_i.h"

using namespace std;
//using namespace boost;
using namespace KC;
