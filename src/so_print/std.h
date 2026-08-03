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

#include <QObject>
#include <QException>
#include <QCoreApplication>
#include <QPrinter>
#include <QPrintEngine>
#include <QTextDocument>
#include <QTextStream>
#include <QDebug>

#include "common/base_type.h"
#include "util/auto_release.h"
#include "util/util_funcs.h"
#include "util/proc_meminfo.h"
#include "util/backtrace_symbols.h"
#include "for_user/kc_controller_i.h"

using namespace std;
using namespace boost;
using namespace KC;
