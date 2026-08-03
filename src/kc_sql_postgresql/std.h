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
#include <sstream>
#include <csignal>
#include <csetjmp>
#include <atomic>
#include <regex>
#include <chrono>
#include <clocale>

// #define BOOST_ALL_DYN_LINK
#include <boost/smart_ptr.hpp>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <libpq-fe.h>
//#include <catalog/pg_type.h>
//#include <server/catalog/pg_type_d.h>
#include "pg_type_define.h"

#include "common/base_type.h"
#include "util/auto_release.h"
#include "util/util_funcs.h"
#include "util/util_funcs_ex.h"
#include "util/ctrl_sql.h"
#include "util/ctrl_script.h"
#include "util/proc_meminfo.h"
#include "util/backtrace_symbols.h"
#include "util/bundle_activator.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_exception.h"
#include "kc_controller/kc_sql_pgsql_i.h"

using namespace std;
using namespace boost;
using namespace KC;
