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
#include <boost/any.hpp>
#include <boost/locale.hpp>
using namespace boost;

#include "soci.h"
using namespace soci;

#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "util/process_name_lock.h"
#include "kc_web/web_struct.h"
#include "for_user/common_define.h"
#include "for_user/page_interface.h"
#include "for_user/lib/db/kc_db_i.h"
#include "util/signo_try.h"
using namespace KC;
