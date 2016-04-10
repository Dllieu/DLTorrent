#include "Logger.h"

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>

namespace bl = boost::log;
namespace ble = bl::expressions;
namespace blk = bl::keywords;

namespace
{
    void    init_console_logger()
    {
        auto consoleSink = bl::add_console_log( std::clog );
        consoleSink->set_formatter( ble::format( "[%1%] %2%" ) % ble::attr<bl::trivial::severity_level>( "Severity" ) % bl::expressions::smessage );
    }

    void    init_file_logger()
    {
        // [TimeStamp] [ThreadId] [Severity Level] [Scope] Log message
        auto fmtTimeStamp = ble::format_date_time<boost::posix_time::ptime>( "TimeStamp", "%Y-%m-%d %H:%M:%S.%f" );
        auto fmtThreadId = ble::attr<bl::attributes::current_thread_id::value_type>( "ThreadID" );
        auto fmtSeverity = ble::attr<bl::trivial::severity_level>( "Severity" );
        auto fmtScope = ble::format_named_scope( "Scope", blk::format = "%n(%f:%l)", blk::iteration = bl::expressions::reverse, blk::depth = 2 );

        auto logFmt = ble::format( "[%1%] (%2%) [%3%] [%4%] %5%" ) % fmtTimeStamp
                                                                   % fmtThreadId
                                                                   % fmtSeverity
                                                                   % fmtScope
                                                                   % ble::smessage;

        /* fs sink */
        auto fsSink = bl::add_file_log( blk::file_name = "test_%Y-%m-%d_%H-%M-%S.%N.log",
                                        blk::rotation_size = 10 * 1024 * 1024,
                                        blk::min_free_space = 30 * 1024 * 1024,
                                        blk::open_mode = std::ios_base::app );
        fsSink->set_formatter( logFmt );
        fsSink->locked_backend()->auto_flush( true );
    }
}

void    utility::Logger::init()
{
    /* init boost log
    * 1. Add common attributes
    * 2. set log filter to trace
    */
    bl::add_common_attributes();
    bl::core::get()->add_global_attribute( "Scope", bl::attributes::named_scope() );
    bl::core::get()->set_filter( bl::trivial::severity >= bl::trivial::trace );

    ::init_console_logger();
    // ::init_file_logger()

    // ????? must be called in every thread???
    BOOST_LOG_FUNCTION();
}
