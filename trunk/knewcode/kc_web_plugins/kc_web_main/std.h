#pragma once
#pragma GCC system_header

#include <string>
#include <stdexcept>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <csignal>
#include <csetjmp>
using namespace std;

#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
using namespace boost;

#include "common/knewcode_version.h"
#include "util/auto_release.h"
#include "util/util_func.h"
#include "util/get_sysinfo.h"
#include "util/proc_meminfo.h"
#include "util/register_signal.h"
#include "util/backtrace_symbols.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/kc_web_work_i.h"
#include "kc_web/page_data_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"
#include "util/signo_try.h"
using namespace KC;
