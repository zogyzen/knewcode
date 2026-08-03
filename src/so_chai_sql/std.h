#pragma once

#include <string>
#include <stdexcept>

#include <boost/thread.hpp>

#include "util/kc_log.h"
// #include "util/kc_log.bak.h"
#include "util/util_funcs.h"
#include "util/backtrace_sigsegv.h"
#include "for_user/kc_controller_i.h"
#include "for_cpp/chai_module_i.h"
#include "kc_controller/kc_chai_script_i.h"

using namespace std;
using namespace boost;
using namespace KC;
