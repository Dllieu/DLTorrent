#pragma once

#include <boost/log/trivial.hpp>

#ifndef LOG_DEBUG
# define LOG_DEBUG BOOST_LOG_TRIVIAL( debug )
#endif

#ifndef LOG_INFO
# define LOG_INFO BOOST_LOG_TRIVIAL( info )
#endif

#ifndef LOG_WARNING
# define LOG_WARNING BOOST_LOG_TRIVIAL( warning )
#endif

#ifndef LOG_ERROR
# define LOG_ERROR BOOST_LOG_TRIVIAL( error )
#endif

#ifndef LOG_FATAL
# define LOG_FATAL BOOST_LOG_TRIVIAL( fatal )
#endif

namespace utility
{
    class Logger
    {
    public:
        static void     init();
    };
}
