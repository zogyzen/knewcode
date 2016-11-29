#pragma once
#pragma GCC system_header

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <stdexcept>
using namespace std;

#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
using namespace boost;

#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "for_user/lib/net/net_action_i.h"
using namespace KC;
