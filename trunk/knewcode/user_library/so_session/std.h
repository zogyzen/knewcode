#pragma once
#pragma GCC system_header

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <stdexcept>
#include <thread>
using namespace std;

#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
using namespace boost;

#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "util/process_name_list.h"
#include "for_user/common_define.h"
#include "for_user/lib/session/kc_session_i.h"
#include "kc_web/keyword_two.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/inner_var_i.h"
#include "kc_web/page_data_i.h"
using namespace KC;

constexpr char const g_SessionConnName[] = "__KCPageSessionConnect19450903___";
