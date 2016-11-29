#pragma once
#pragma GCC system_header

#include <functional>
#include <fstream>
#include <deque>
#include <string>
#include <string.h>
#include <algorithm>
#include <stdexcept>
using namespace std;

#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/any.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/format.hpp>
using namespace boost;
using namespace boost::spirit;
using namespace boost::phoenix;

#include "util/auto_release.h"
#include "util/kc_tuple.h"
#include "framework/bundle_activator_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/parse_work_i.h"
#include "kc_web/web_struct_gram.h"
#include "kc_web/syntax_struct_one.h"
#include "kc_web/syntax_struct_two.h"
#include "kc_web/keyword_one.h"
#include "kc_web/keyword_two.h"
using namespace KC;
using namespace KC::Keychar::one_level;
using namespace KC::Keychar::two_level;
