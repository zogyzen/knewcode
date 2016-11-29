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

#include "SOAPParser.h"
#include "SOAPElement.h"
#include "SOAPEncoder.h"
#include "SOAPMethod.h"

#include "util/auto_release.h"
#include "util/load_library.h"
#include "util/util_func.h"
#include "common/base_type.h"
#include "kc_web/keyword_one.h"
#include "for_user/lib/service/kc_webservice_i.h"
using namespace KC;
