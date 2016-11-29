#pragma once
#pragma GCC system_header

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
using namespace std;

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
using namespace boost;

#include "common/knewcode_version.h"
#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "util/proc_meminfo.h"
#include "util/register_signal.h"
#include "util/temp_log.h"
#include "util/lock_work.h"
#include "framework/framework_i.h"
#include "framework/bundle_activator_i.h"
#include "framework/service_registration_i.h"
#include "framework_ex/bundle_context_ex_i.h"
#include "framework_ex/bundle_ex_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
using namespace KC;
