#include "kc_log.h"

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/log/common.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/detail/format.hpp>
#include <boost/log/detail/process_id.hpp>
#include <boost/log/detail/thread_id.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

using boost::shared_ptr;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)
BOOST_LOG_ATTRIBUTE_KEYWORD(log_severity, "Severity", logging::trivial::severity_level)

boost::log::sources::severity_logger<boost::log::trivial::severity_level> KC::MyLog::s_slg;

typedef sinks::text_file_backend backend_t;
typedef sinks::asynchronous_sink<
    backend_t,
    sinks::unbounded_ordering_queue<
        logging::attribute_value_ordering<unsigned int, std::less<unsigned int>>
    >
> sink_t;
typedef shared_ptr<sink_t> sink_tptr;

void KC::MyLog::Init(const char* dir, boost::log::trivial::severity_level lv)
{
    if (!boost::filesystem::exists(dir))
        boost::filesystem::create_directories(dir);

    sink_tptr sink(new sink_t(boost::make_shared<backend_t>(), keywords::order = logging::make_attr_ordering("RecordID", std::less<unsigned int>())));
    sink->locked_backend()->set_rotation_size(1024 * 1024);
    sink->locked_backend()->set_file_name_pattern(string(dir) + "/%Y%m%d-" + boost::lexical_cast<std::string>(getpid()) + ".log");
    sink->locked_backend()->set_file_collector(sinks::file::make_collector(
        keywords::target = string(dir) + "/back/" + boost::lexical_cast<std::string>(getpid())        //备份文件夹名
        //, keywords::max_size = 50 * 1024 * 1024           // 文件夹所占最大空间
        //, keywords::min_free_space = 100 * 1024 * 1024    // 磁盘最小预留空间
    ));
    sink->locked_backend()->auto_flush(true);
    sink->locked_backend()->scan_for_files();

    sink->set_filter(log_severity >= lv);
    //sink->imbue(std::locale("zh_CN.UTF-8"));   // 本地化
    sink->set_formatter
    (
        expr::stream
        << "[№" << expr::attr<unsigned int>("RecordID")
        << " §" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
        << " ※" << logging::trivial::severity
        << "] " << expr::smessage
        << "[" << expr::attr<logging::aux::process::id>("ProcessID") << ":" << expr::attr<logging::aux::thread::id>("ThreadID") << "]\n"
    );

    logging::add_common_attributes();

    // Add it to the core
    logging::core::get()->add_sink(sink);

    // Add some attributes too
    logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
    logging::core::get()->add_global_attribute("RecordID", attrs::counter<unsigned int>(1));
}
