//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "utility/GenericBuffer.h"

using namespace utility;

BOOST_AUTO_TEST_SUITE( GenericBufferTestSuite )

BOOST_AUTO_TEST_CASE( IntegralTest )
{
    GenericBuffer< 256 > buffer;

    auto ll = static_cast< long long >( 54546541123 );
    auto i = 123;
    auto ni = -65465;
    char c = 4;

    buffer << ll << i << ni << c;

    long long ll2 = 0;
    int i2 = 0;
    int ni2 = 0;
    char c2 = 0;

    buffer >> ll2 >> i2 >> ni2 >> c2;

    BOOST_CHECK( ll == ll2 );
    BOOST_CHECK( i == i2 );
    BOOST_CHECK( ni == ni2 );
    BOOST_CHECK( c == c2 );
}

namespace
{
    struct POD_message_foo
    {
        long long   ll;
        int         i;

        bool    operator==( const POD_message_foo& m ) { return ll == m.ll && i == m.i; }
    };
}

BOOST_AUTO_TEST_CASE( PodTest )
{
    GenericBuffer< 256 > buffer;

    POD_message_foo fooFrom{ 56411025, -9441 };
    buffer.write_pod( fooFrom );

    POD_message_foo fooTo;
    buffer >> fooTo;

    BOOST_CHECK( fooFrom == fooTo );
}

BOOST_AUTO_TEST_SUITE_END() // GenericBufferTestSuite
