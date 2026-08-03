#pragma once
#ifdef __GNUC__
#pragma GCC system_header
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

#include <boost/smart_ptr.hpp>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <chaiscript/chaiscript.hpp>

#include "common/base_type.h"
#include "util/auto_release.h"
#include "util/util_funcs_ex.h"
#include "util/ctrl_sql.h"
#include "util/ctrl_script.h"
#include "util/proc_meminfo.h"
#include "util/backtrace_symbols.h"
#include "util/bundle_activator.h"
#include "util/load_sub_lib.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"
#include "kc_controller/kc_chai_script_i.h"
#include "for_cpp/chai_module_i.h"

using namespace std;
using namespace boost;
using namespace KC;
