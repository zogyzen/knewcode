#pragma once
#ifdef __GNUC__
#pragma GCC system_header
#endif

#include <string>
#include <stdexcept>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <csignal>
#include <csetjmp>

#include <boost/smart_ptr.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "for_user/common_define.h"
#include "for_user/kc_controller_i.h"
#include "for_user/page_interface.h"

using namespace std;
using namespace boost;
using namespace KC;
