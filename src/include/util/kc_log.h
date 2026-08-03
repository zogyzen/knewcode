#pragma once

#include <iostream>
#include <string>

//#undef _WIN32_WINNT
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/process.hpp>
#include <boost/log/trivial.hpp>
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

typedef sinks::text_file_backend backend_t;
typedef sinks::asynchronous_sink<
    backend_t,
    sinks::unbounded_ordering_queue<logging::attribute_value_ordering<unsigned int, std::less<unsigned int>>>
> sink_t;
typedef boost::shared_ptr<sink_t> sink_tptr;

namespace KC
{
    class MyLog
    {
    public:
        // 在使用之前必须先调用此函数
        void Init(std::string dir, std::string id, boost::log::trivial::severity_level lv, int szFile = 2, std::string eachPrefix = "", bool inDay = false)
        {
            std::cout << "*[knewcode] MyLog::Init Begin (" << lv << "): " << dir << " | " << id << std::endl;
            if (!boost::filesystem::exists(dir)) boost::filesystem::create_directories(dir);
            // string exePath = boost::filesystem::initial_path<boost::filesystem::path>().leaf().string();
            //sink_tptr sink(new sink_t(boost::make_shared<backend_t>(), keywords::order = logging::make_attr_ordering("RecordID", std::less<unsigned int>())));
            //m_sink.reset(new sink_t(boost::make_shared<backend_t>(), keywords::order = logging::make_attr_ordering("RecordID", std::less<unsigned int>())));
            auto odAttr = logging::make_attr_ordering("RecordID", std::less<unsigned int>());
            auto sinkBack = boost::make_shared<backend_t>();
            //auto sinkBack = new backend_t;
            auto sinkPtr = new sink_t(sinkBack,
                keywords::order = odAttr,
                // keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),  // 每天轮转
                keywords::min_free_space = 1.5 * 1024 * 1024 * 1024  // 最低磁盘空间限制
            );
            if (nullptr == sinkPtr) throw std::runtime_error("Can't new boost::log::sinks::asynchronous_sink.");
            m_sink.reset(sinkPtr);
            // std::cout << 1 << std::endl;
            auto logLockPtr = m_sink->locked_backend();
            std::cout << " \t\t MyLog locked backend" << std::endl;
            if (logLockPtr.get() != nullptr)
            {
                logLockPtr->set_open_mode(std::ios_base::app);
                logLockPtr->set_rotation_size(1024 * 1024 * szFile);
                if (inDay)
                    logLockPtr->set_file_name_pattern(dir + "/" + id + "_%Y%m%d_%N.log");
                else
                    logLockPtr->set_file_name_pattern(dir + "/" + id + "_%Y%m%d%H%M%S_%N_" + std::to_string(boost::this_process::get_id()) + ".log");
                // std::cout << 3 << std::endl;
                logLockPtr->set_file_collector(sinks::file::make_collector(
                    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),  // 每天轮转
                    keywords::target = dir + "/bak/" + FmtPTime("%H")        //备份文件夹名
                    //, keywords::max_size = 50 * 1024 * 1024           // 文件夹所占最大空间
                    //, keywords::min_free_space = 100 * 1024 * 1024    // 磁盘最小预留空间
                    ));
                // std::cout << 4 << std::endl;
                logLockPtr->auto_flush(true);
                //logLockPtr->scan_for_files();
            }
            else throw std::runtime_error("sink_t::locked_backend is nullptr");

            m_sink->set_filter(log_severity >= lv);
            // m_sink->imbue(std::locale("zh_CN.UTF-8"));   // 本地化
            m_sink->set_formatter
            (
                expr::stream << eachPrefix
                    << "[#" << expr::attr<unsigned>("RecordID")
                    << " &" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                    // << " @" << expr::attr<attrs::current_process_id::value_type>("ProcessID")
                    << " @" << expr::attr<logging::aux::process::id>("ProcessID") << ":" << expr::attr<logging::aux::thread::id>("ThreadID")
                    << " *" << logging::trivial::severity
                    << "] " << expr::smessage << std::endl
            );

            logging::add_common_attributes();

            // Add it to the core
            logging::core::get()->add_sink(m_sink);

            // Add some attributes too
            logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
            logging::core::get()->add_global_attribute("RecordID", attrs::counter<unsigned int>(1));
            std::cout << "*[knewcode] MyLog::Init Success." << std::endl;
        }

        // 结束后调用此函数
        void Free(void)
        {
            m_sink->stop();
            m_sink.reset();

            logging::core::get()->flush();
            logging::core::get()->remove_all_sinks();

            //logging::core::get()->remove_sink(m_sink);
            //m_sink->stop();
            //m_sink->flush();
            //m_sink.reset();
        }

        // 写日志
        void WriteLog(std::string sMsg, boost::log::trivial::severity_level level = boost::log::trivial::info)
        {
            BOOST_LOG_SEV(s_slg, level) << sMsg;
        }

        // 提交
        void Flush(void) const
        {
            try
            {
                logging::core::get()->flush();
            }
            catch (...) {}
        }

        // 时间格式
        std::string FmtPTime(std::string sFmt = "%Y-%m-%d %H:%M:%S.%f", boost::posix_time::ptime time = boost::posix_time::second_clock::local_time())
        {
            boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
            facet->format(sFmt.c_str());
            std::stringstream stream;
            stream.imbue(std::locale(std::locale::classic(), facet));
            stream << time;
            return stream.str();
        }

    public:
        boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> s_slg;
        sink_tptr m_sink;
    };
}

#define MY_BOOST_LOG_SEV(logger, lvl) BOOST_LOG_SEV((logger.s_slg), lvl)
#define MY_BOOST_LOG_SEV_DEBUG(logger) MY_BOOST_LOG_SEV(logger, boost::log::trivial::debug)
#define MY_BOOST_LOG_SEV_LOG(logger) MY_BOOST_LOG_SEV(logger, boost::log::trivial::info)
#define MY_BOOST_LOG_SEV_ERR(logger) MY_BOOST_LOG_SEV(logger, boost::log::trivial::error)
