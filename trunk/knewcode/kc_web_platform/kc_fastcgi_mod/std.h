#pragma once
#pragma GCC system_header

#include <string>
#include <stdexcept>
using namespace std;

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/thread.hpp>
using namespace boost;

#include "common/knewcode_version.h"
#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "util/auto_release.h"
#include "util/several_thread.h"
#include "framework/framework_i.h"
#include "framework/bundle_context_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/kc_web_work_i.h"
using namespace KC;

#include "fcgi_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgio.h"
