#pragma once
#pragma GCC system_header

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace std;

#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
using namespace boost;

#include "util/auto_release.h"
#include "util/util_func.h"
#include "util/data_separator.h"
#include "framework/bundle_activator_i.h"
#include "kc_web/inner_var_i.h"
#include "kc_web/page_data_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"
#include "kc_web/keyword_one.h"
#include "kc_web/keyword_two.h"
using namespace KC;
using namespace KC::Keychar::one_level;
using namespace KC::Keychar::two_level;
