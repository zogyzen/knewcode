#pragma once
#ifdef __GNUC__
    #pragma GCC system_header
#endif

#if defined(__MINGW32__)
    #ifndef __kernel_entry
        #define __kernel_entry
    #endif
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

#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/process.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>

#include "cJSON/src/cJSON.h"

#include "common/base_type.h"
#include "util/auto_release.h"
#include "util/util_funcs.h"
#include "util/ctrl_script.h"
#include "util/proc_meminfo.h"
#include "util/backtrace_symbols.h"
#include "util/request_respond_tmp.h"
#include "util/bundle_activator.h"
#include "framework/bundle_activator_i.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/web_exception.h"
#include "kc_web/web_struct.h"
#include "kc_web/kc_main_work_i.h"
#include "kc_web/kc_web_work_i.h"
#include "kc_controller/kc_h5_sse_ws_i.h"
#include "kc_controller/kc_session_cookie_i.h"

using namespace std;
using namespace boost;
using namespace KC;
