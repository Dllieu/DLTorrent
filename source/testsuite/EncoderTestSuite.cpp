//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <iostream>

#include "parsing/BDecoder.h"
#include "parsing/BEncoder.h"

using namespace parsing;

BOOST_AUTO_TEST_SUITE( EncoderTestSuite )

BOOST_AUTO_TEST_CASE( EncodeIntegerTest )
{
    BOOST_CHECK( BEncoder::encode( 5 ) == "i5e" );
    BOOST_CHECK( BEncoder::encode( -0 ) == "i0e" );
    BOOST_CHECK( BEncoder::encode( -15 ) == "i-15e" );
}

BOOST_AUTO_TEST_CASE( EncodeStringTest )
{
    BOOST_CHECK( BEncoder::encode( "" ) == "0:" );
    BOOST_CHECK( BEncoder::encode( "hi" ) == "2:hi" );
}

BOOST_AUTO_TEST_CASE( EncodeListTest )
{
    std::vector< Metadata > v { 7, 2, "str" };
    BOOST_CHECK( BEncoder::encode( v ) == "li7ei2e3:stre" );
}

namespace
{
    // can only test dictionary with 2 keys max
    bool    encodeDecodeEncodeDecode( const std::string& encodedMetadata )
    {
        auto firstPass = BEncoder::encode( BDecoder::decode( encodedMetadata ) );
        auto secondPass = BEncoder::encode( BDecoder::decode( firstPass ) );

        return firstPass == secondPass || secondPass == encodedMetadata || firstPass == encodedMetadata;
    }
}

// Dictionary are implemented with an unordered_map, the key can't always match the one from the encoding metadata (would have the same problem with std::map)
BOOST_AUTO_TEST_CASE( EncodeDictionaryTest )
{
    std::unordered_map< std::string, Metadata > m { { "key", "value" } };
    BOOST_CHECK( BEncoder::encode( m ) == "d3:key5:valuee" );
    BOOST_CHECK( encodeDecodeEncodeDecode( "d3:key5:value3:KEYli5e3:supee" ) );
}

BOOST_AUTO_TEST_SUITE_END() // ! EncoderTestSuite
