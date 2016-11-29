#pragma once
#pragma GCC system_header

#include <vector>
#include <string>
#include <stdexcept>
#include <csetjmp>
using namespace std;

#include <boost/any.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
using namespace boost;

#include "util/auto_release.h"
#include "util/util_func.h"
#include "framework/bundle_activator_i.h"
#include "kc_web/dyn_call_i.h"
#include "kc_web/page_data_i.h"
#include "kc_web/web_struct.h"
#include "lib_func_dyncall/kc_func_dyncall.h"
#include "util/signo_try.h"
using namespace KC;
