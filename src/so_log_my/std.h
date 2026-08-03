#pragma once

#include <string>
#include <stdexcept>

#include <boost/thread.hpp>

#include "util/kc_log.h"
// #include "util/kc_log.bak.h"
#include "util/util_funcs.h"
#include "util/backtrace_sigsegv.h"
#include "for_user/kc_controller_i.h"
#include "framework/func_log_i.h"
#include "framework/bundle_context_i.h"

using namespace std;
using namespace boost;
using namespace KC;
