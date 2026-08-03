#pragma once

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/process.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/keywords/order.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
typedef sinks::asynchronous_sink<sinks::text_file_backend> file_sink_t;
typedef boost::shared_ptr<file_sink_t> file_sink_ptr_t;

namespace KC
{
    class MyLog
    {
    public:
        void Init(std::string dir, std::string id, boost::log::trivial::severity_level lv, int szFile = 2, std::string eachPrefix = "", bool inDay = false)
        {
            std::cout << "*[knewcode] MyLog::Init Begin (" << lv << "): " << dir << std::endl;
            if (!boost::filesystem::exists(dir)) boost::filesystem::create_directories(dir);
            // 添加公共属性（如时间戳、线程ID）
            logging::add_common_attributes();
            logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());
            // 序号
            auto odAttr = logging::make_attr_ordering("RecordID", std::less<unsigned int>());

            // 定义日志格式：[时间戳][线程 ID][级别]: 消息
            auto fmt = expr::stream << std::dec << eachPrefix
                    << "[#" << expr::attr<unsigned>("RecordID")
                    << " &" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "]"
                    << " @" << expr::attr<attrs::current_process_id::value_type>("ProcessID")
                    << ":" << expr::attr<attrs::current_thread_id::value_type>("ThreadID")
                    << " *" << logging::trivial::severity << "]"
                    << expr::smessage << std::endl;

            // 文件接收器（异步）
            m_sink = boost::make_shared<file_sink_t>(
                keywords::order = odAttr,
                keywords::file_name = inDay
                    ? dir + "/" + id + "_%Y%m%d_%N.log"  // 文件名：x_20250609_000.log
                    : dir + "/" + (boost::format("%s_%d") % id % boost::this_process::get_id()).str() + "_%Y%m%d%H%M%S_%N.log",
                keywords::rotation_size = szFile * 1024 * 1024,  // 2 MB 轮转
                keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),  // 每天轮转
                keywords::min_free_space = 1.5 * 1024 * 1024 * 1024  // 最低磁盘空间限制
            );
            m_sink->set_formatter(fmt);
            m_sink->set_filter(logging::trivial::severity >= lv);
            logging::core::get()->add_sink(m_sink);

            // Add some attributes too
            logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
            logging::core::get()->add_global_attribute("RecordID", attrs::counter<unsigned int>(1));

            // 控制台接收器（异步）
            // typedef sinks::asynchronous_sink<sinks::text_ostream_backend> console_sink;
            // auto console_sink_ptr = boost::make_shared<console_sink>();
            // console_sink_ptr->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
            // console_sink_ptr->set_formatter(fmt);
            // console_sink_ptr->set_filter(logging::trivial::severity >= logging::trivial::info);
            // logging::core::get()->add_sink(console_sink_ptr);

            std::cout << "*[knewcode] MyLog::Init Success." << std::endl;
        }

        // 结束后调用此函数
        void Free(void)
        {
            try
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
            catch (...) {}
        }

        // 写日志
        // void WriteLog(std::string sMsg, boost::log::trivial::severity_level level = boost::log::trivial::info)
        // {
        //     BOOST_LOG_TRIVIAL(info) << sMsg;
        // }

        // 提交
        void Flush(void) const
        {
            try
            {
                logging::core::get()->flush();
            }
            catch (...) {}
        }

    public:
        boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> s_slg;
        file_sink_ptr_t m_sink;
    };
}

#define MY_BOOST_LOG_SEV(mylog, lvl) \
    BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(), \
        (::boost::log::keywords::severity = lvl))
// #define MY_BOOST_LOG_SEV(logger, lvl) BOOST_LOG_SEV((logger.s_slg), lvl)
