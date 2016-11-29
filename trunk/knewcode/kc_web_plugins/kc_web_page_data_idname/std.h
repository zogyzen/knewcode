#pragma once
#pragma GCC system_header

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <stdexcept>
using namespace std;

#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
using namespace boost;

#include "util/auto_release.h"
#include "util/util_func.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_struct.h"
#include "kc_web/syntax_struct_two.h"
#include "kc_web/work_idname_item_i.h"
#include "kc_web/work_expr_tree_i.h"
#include "kc_web/page_data_i.h"
#include "kc_web/dyn_call_i.h"
using namespace KC;
