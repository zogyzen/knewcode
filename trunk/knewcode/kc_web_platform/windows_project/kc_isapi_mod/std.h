#pragma once

#include <string>
#include <stdexcept>
using namespace std;

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
using namespace boost;

#include "isapi_struct.h"

#include "util/util_func.h"
#include "framework/framework_i.h"
#include "framework/bundle_context_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/kc_web_work_i.h"
using namespace KC;
