//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "utility/GenericBigEndianBuffer.h"

using namespace utility;

BOOST_AUTO_TEST_SUITE( GenericBigEndianBufferTestSuite )

BOOST_AUTO_TEST_CASE( IntegralTest )
{
    GenericBigEndianBuffer< 256 > buffer;

    auto ll = static_cast< long long >( 54546541123 );
    auto i = 123;
    auto ni = -65465;
    char c = 4;
    auto b = true;

    buffer << ll << i << ni << c << b;

    long long ll2 = 0;
    int i2 = 0;
    int ni2 = 0;
    char c2 = 0;
    bool b2 = false;

    buffer >> ll2 >> i2 >> ni2 >> c2 >> b2;

    BOOST_CHECK( ll == ll2 );
    BOOST_CHECK( i == i2 );
    BOOST_CHECK( ni == ni2 );
    BOOST_CHECK( c == c2 );
    BOOST_CHECK( b == b2 );

    buffer << i;
    auto i3 = static_cast< int >( buffer );

    BOOST_CHECK( i == i3 );
}

BOOST_AUTO_TEST_CASE( StringTestSuite )
{
    GenericBigEndianBuffer< 256 > buffer;

    std::string s1( "hello" );
    buffer.writeString( s1, s1.size() );

    std::string s2;
    buffer.readString( s2, s1.size() ); // no need to take the '\0'

    BOOST_CHECK( s1 == s2 );

    buffer.writeString( "world", 50 );
    buffer.readString( s2, 50 );

    BOOST_CHECK( s2 == "world" );
}

BOOST_AUTO_TEST_SUITE_END() // GenericBigEndianBufferTestSuite
