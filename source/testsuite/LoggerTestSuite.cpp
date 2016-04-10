//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "utility/Logger.h"

BOOST_AUTO_TEST_SUITE( LoggerTestSuite )

BOOST_AUTO_TEST_CASE( LoggerTest )
{
    utility::Logger::init();

    LOG_DEBUG   << "DEBUG";
    LOG_INFO    << "INFO";
    LOG_WARNING << "WARNING";
    LOG_ERROR   << "ERROR";
    LOG_FATAL   << "FATAL";

    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // ! LoggerTestSuite
