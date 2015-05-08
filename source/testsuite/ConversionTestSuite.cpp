//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "utility/Conversion.h"

using namespace utility;

BOOST_AUTO_TEST_SUITE( ConversionTestSuite )

BOOST_AUTO_TEST_CASE( NaiveUIntTest )
{
    BOOST_CHECK_THROW( naive_uint_conversion( "5a" ), std::invalid_argument );

    BOOST_CHECK( naive_uint_conversion( "54" ) == 54 );
}

BOOST_AUTO_TEST_CASE( NaiveLLTest )
{
    BOOST_CHECK_THROW( naive_ll_conversion( "-++-5+" ), std::invalid_argument );

    BOOST_CHECK( naive_ll_conversion( "--++-+-+5" ) == 5 );
    BOOST_CHECK( naive_ll_conversion( "--++--+-+5" ) == -5 );
}

BOOST_AUTO_TEST_SUITE_END() // ! ConversionTestSuite
