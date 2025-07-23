#include "init_log.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <iostream>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

void init_log() {
    // 添加公共属性（如时间戳、线程 ID）
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());

    // 定义日志格式：[时间戳][线程 ID][级别]: 消息
    auto fmt = expr::stream
        << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "]"
        << "[" << expr::attr<attrs::current_thread_id::value_type>("ThreadID") << "]"
        << "[" << logging::trivial::severity << "]"
        << ": " << expr::smessage;

    // 文件接收器（异步）
    typedef sinks::asynchronous_sink<sinks::text_file_backend> file_sink;
    auto file_sink_ptr = boost::make_shared<file_sink>(
        keywords::file_name = "app_%Y%m%d_%N.log",  // 文件名：app_20250609_000.log
        keywords::rotation_size = 10 * 1024 * 1024,  // 10 MB 轮转
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)  // 每天轮转
    );
    file_sink_ptr->set_formatter(fmt);
    file_sink_ptr->set_filter(logging::trivial::severity >= logging::trivial::info);
    logging::core::get()->add_sink(file_sink_ptr);

    // 控制台接收器（异步）
    typedef sinks::asynchronous_sink<sinks::text_ostream_backend> console_sink;
    auto console_sink_ptr = boost::make_shared<console_sink>();
    console_sink_ptr->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
    console_sink_ptr->set_formatter(fmt);
    console_sink_ptr->set_filter(logging::trivial::severity >= logging::trivial::info);
    logging::core::get()->add_sink(console_sink_ptr);
}



/* use:
init_log();
BOOST_LOG_TRIVIAL(trace) << "This is a trace message (filtered out)";
BOOST_LOG_TRIVIAL(debug) << "This is a debug message (filtered out)";
BOOST_LOG_TRIVIAL(info) << "This is an info message";
BOOST_LOG_TRIVIAL(warning) << "This is a warning message";
BOOST_LOG_TRIVIAL(error) << "This is an error message";
BOOST_LOG_TRIVIAL(fatal) << "This is a fatal message";
*/