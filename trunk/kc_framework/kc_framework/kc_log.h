#pragma once

#include <string>
#include <boost/log/trivial.hpp>
using std::string;

#define KC_WRITE_LOG(level) BOOST_LOG_SEV((MyLog::s_slg),(level))

// 在使用之前必须先调用 init
// 使用方式  KC_WRITE_LOG(log::trivial::info) << "test string";

namespace KC
{
    class MyLog
    {
    public:
      MyLog() = delete;

      // 在使用之前必须先调用此函数
      static void Init(const char*, boost::log::trivial::severity_level);
      static boost::log::sources::severity_logger<boost::log::trivial::severity_level> s_slg;
    };
}
