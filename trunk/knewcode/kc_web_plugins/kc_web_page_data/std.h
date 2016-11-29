#pragma once
#pragma GCC system_header

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <stdexcept>
using namespace std;

#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
using namespace boost;

#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "util/proc_meminfo.h"
#include "util/lock_work.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_struct.h"
#include "kc_web/inner_var_i.h"
#include "kc_web/page_data_i.h"
#include "kc_web/parse_work_i.h"
#include "kc_web/work_syntax_item_i.h"
#include "kc_web/work_expr_tree_i.h"
#include "kc_web/syntax_struct_two.h"
#include "kc_web/keyword_one.h"
#include "kc_web/keyword_two.h"
#include "util/signo_try.h"
using namespace KC;
using namespace KC::Keychar::one_level;
using namespace KC::Keychar::two_level;
