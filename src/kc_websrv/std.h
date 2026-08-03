#pragma once
#ifdef __GNUC__
    #pragma GCC system_header
#endif

#if defined(__MINGW32__)
    #ifndef __kernel_entry
        #define __kernel_entry
    #endif
#endif

#include <iostream>
#include <memory>

#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "common/knewcode_version.h"
#include "kc_web/kc_websrv_proxy_i.h"
#include "util/load_websrv_proxy.h"
#include "util/load_websrv_api.h"
#include "util/util_funcs.h"
#include "util_websrv/kc_srv.h"
#include "framework_ex/bundle_context_ex_i.h"

using namespace std;
using namespace boost;
using namespace KC;
