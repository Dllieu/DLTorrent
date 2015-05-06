//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/utility/string_ref.hpp>

#include "parsing/Metadata.h"
#include "parsing/BDecoder.h"

using namespace parsing;

BOOST_AUTO_TEST_SUITE( DecoderTestSuite )

BOOST_AUTO_TEST_CASE( DecodeStringTest )
{
    BOOST_CHECK_THROW( BDecoder::decode( "6:salut" ), std::exception );

    BOOST_CHECK( boost::get< std::string >( BDecoder::decode( "5:salut" ) ) == "salut" );
    BOOST_CHECK( boost::get< std::string >( BDecoder::decode( "0:" ) ) == "" );
}

BOOST_AUTO_TEST_CASE( DecodeIntegerTest )
{
    BOOST_CHECK_THROW( BDecoder::decode( "ie" ), std::exception );
    BOOST_CHECK_THROW( BDecoder::decode( "i" ), std::exception );

    BOOST_CHECK( boost::get< long long >( BDecoder::decode( "i5e" ) ) == 5 );
    BOOST_CHECK( boost::get< long long >( BDecoder::decode( "i-5e" ) ) == -5 );
}

BOOST_AUTO_TEST_CASE( DecodeListTest )
{
    BOOST_CHECK( boost::get< std::vector< Metadata > >( BDecoder::decode( "le" ) ).empty() );
    BOOST_CHECK( boost::get< std::vector< Metadata > >( BDecoder::decode( "li5e5:salute" ) ).size() == 2 );
}

BOOST_AUTO_TEST_CASE( DecodeDictionaryTest )
{
    BOOST_CHECK_THROW( BDecoder::decode( "d3:keye" ), std::exception );

    using MetadataDictionary = std::unordered_map< std::string, Metadata >;
    BOOST_CHECK( boost::get< MetadataDictionary >( BDecoder::decode( "de" ) ).empty() );
    BOOST_CHECK( boost::get< MetadataDictionary >( BDecoder::decode( "d3:key5:valuee" ) ).size() == 1 );
    BOOST_CHECK( boost::get< MetadataDictionary >( BDecoder::decode( "d3:key5:value4:key2li5e3:supee" ) ).size() == 2 );
    BOOST_CHECK( boost::get< MetadataDictionary >( BDecoder::decode( "d3:KEYli5e3:supe3:key5:valuee" ) ).size() == 2 );
}

BOOST_AUTO_TEST_SUITE_END() // ! DecoderTestSuite
