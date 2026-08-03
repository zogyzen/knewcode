#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <typeinfo>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>

#include "for_user/kc_controller_i.h"
#include "for_cpp/mq_callback_helper.h"
#include "util/util_funcs.h"
#include "util/backtrace_symbols.h"

using namespace std;
using namespace KC;
