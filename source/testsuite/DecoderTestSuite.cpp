//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "parsing/BDecoder.h"

using namespace parsing;

BOOST_AUTO_TEST_SUITE( DecoderTestSuite )

BOOST_AUTO_TEST_CASE( DecodeStringTest )
{
    BOOST_CHECK_THROW( BDecoder::decodeNonBinary( "6:salut" ), std::exception );

    BOOST_CHECK( boost::get< std::string >( BDecoder::decodeNonBinary( "5:salut" ) ) == "salut" );
    BOOST_CHECK( boost::get< std::string >( BDecoder::decodeNonBinary( "0:" ) ) == "" );

    const char s[] = "4:\012\0";
    std::string MetaInfoWithBinary( s, s + 6 );
    auto resultBinary = boost::get< std::string >( BDecoder::decode( MetaInfoWithBinary ) );
    BOOST_CHECK( resultBinary.size() == 4 );
}

BOOST_AUTO_TEST_CASE( DecodeIntegerTest )
{
    BOOST_CHECK_THROW( BDecoder::decodeNonBinary( "ie" ), std::exception );
    BOOST_CHECK_THROW( BDecoder::decodeNonBinary( "i" ), std::exception );

    BOOST_CHECK( boost::get< long long >( BDecoder::decodeNonBinary( "i5e" ) ) == 5 );
    BOOST_CHECK( boost::get< long long >( BDecoder::decodeNonBinary( "i-5e" ) ) == -5 );
}

BOOST_AUTO_TEST_CASE( DecodeListTest )
{
    BOOST_CHECK( boost::get< std::vector< MetaInfo > >( BDecoder::decodeNonBinary( "le" ) ).empty() );
    BOOST_CHECK( boost::get< std::vector< MetaInfo > >( BDecoder::decodeNonBinary( "li5e5:salute" ) ).size() == 2 );
}

BOOST_AUTO_TEST_CASE( DecodeDictionaryTest )
{
    BOOST_CHECK_THROW( BDecoder::decodeNonBinary( "d3:keye" ), std::exception );

    BOOST_CHECK( boost::get< MetaInfoDictionary >( BDecoder::decodeNonBinary( "de" ) ).empty() );
    BOOST_CHECK( boost::get< MetaInfoDictionary >( BDecoder::decodeNonBinary( "d3:key5:valuee" ) ).size() == 1 );
    BOOST_CHECK( boost::get< MetaInfoDictionary >( BDecoder::decodeNonBinary( "d3:key5:value4:key2li5e3:supee" ) ).size() == 2 );
    BOOST_CHECK( boost::get< MetaInfoDictionary >( BDecoder::decodeNonBinary( "d3:KEYli5e3:supe3:key5:valuee" ) ).size() == 2 );
}

BOOST_AUTO_TEST_SUITE_END() // ! DecoderTestSuite
