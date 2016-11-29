#pragma once
#pragma GCC system_header

#include <iostream>
#include <string>
#include <thread>
#include <stdexcept>
using namespace std;

#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>
using namespace boost;

#include "util/load_library.h"
#include "util/util_func.h"
#include "framework/framework_i.h"
#include "framework/bundle_context_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/kc_web_work_i.h"
using namespace KC;
