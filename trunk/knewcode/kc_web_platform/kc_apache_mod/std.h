#pragma once
#pragma GCC system_header

#include <string>
#include <stdexcept>
#include <csignal>
using namespace std;

#define AP_HAVE_DESIGNATED_INITIALIZER

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
using namespace boost;

#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <ap_config.h>

#include "common/knewcode_version.h"
#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "framework/framework_i.h"
#include "framework/bundle_context_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/kc_web_work_i.h"
using namespace KC;
