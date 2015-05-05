//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <boost/utility/string_ref.hpp> 
#include <boost/variant.hpp>
#include <sstream>
#include <iostream>

#include "dltorrent/MetadataValueType.h"

using namespace dltorrent;

BOOST_AUTO_TEST_SUITE( DecoderTestSuite )

namespace
{
    unsigned int    naive_uint_conversion( const boost::string_ref& stringRef )
    {
        unsigned int result = 0;
        for ( auto c : stringRef )
        {
            if ( c < '0' || c > '9' )
                throw std::invalid_argument( std::string( __FUNCTION__ ) + "with " + std::string( stringRef ) );

            result = result * 10 + c - '0';
        }
        return result;
    }

    long long   naive_ll_conversion( const boost::string_ref& stringRef )
    {
        auto finishReadSign = false;
        auto isNegative = false;

        long long result = 0;
        for ( auto c : stringRef )
        {
            if ( !finishReadSign && ( c == '-' || c == '+' ) )
            {
                if ( c == '-' )
                    isNegative = !isNegative;
                continue;
            }

            finishReadSign = true;
            if ( c < '0' || c > '9' )
                throw std::invalid_argument( std::string( __FUNCTION__ ) + "with " + std::string( stringRef ) );

            result = result * 10 + c - '0';
        }
        if ( ! finishReadSign )
            throw std::invalid_argument( std::string( __FUNCTION__ ) + "with " + std::string( stringRef ) );

        return ( isNegative ? -1 : 1 ) * result;
    }

    // expecting: "n:string" with n == string.size()
    std::string     decode_string( boost::string_ref& stringRef )
    {
        assert( !stringRef.empty() && '0' <= stringRef.front() && stringRef.front() <= '9' );

        auto colonIndex = stringRef.find_first_of( ':' );
        if ( colonIndex == std::string::npos )
            throw std::invalid_argument( "Missing ':' after string declaration" );

        auto sizeString = naive_uint_conversion( stringRef.substr( 0, colonIndex ) );

        size_t i = colonIndex + 1;
        if ( i + sizeString > stringRef.size() )
            throw std::invalid_argument( "Out of bound trying to parse a string" );

        auto result = std::string( stringRef.substr( i, sizeString ) );
        i += sizeString;
        stringRef = stringRef.substr( i, stringRef.size() - i );
        return result;
    }

    // expecting: "ine"
    // The initial i and trailing e are beginning and ending delimiters. You can have negative numbers such as i-3e.
    // Only the significant digits should be used, one cannot pad the Integer with zeroes. such as i04e. However, i0e is valid
    // handle n as a signed 64bit integer is mandatory to handle "large files" aka .torrent for more that 4Gbyte
    long long     decode_naive_integer( boost::string_ref& stringRef )
    {
        assert( ! stringRef.empty() && stringRef.front() == 'i' );

        auto endIndex = stringRef.find_first_of( 'e' );
        if ( endIndex == std::string::npos )
            throw std::invalid_argument( "Missing 'e' after integer declaration" );

        if ( endIndex == 1 )
            throw std::invalid_argument( "Empty integer" );

        auto result = naive_ll_conversion( stringRef.substr( 1, endIndex - 1 ) );
        stringRef = stringRef.substr( endIndex + 1, stringRef.size() - endIndex - 1 );
        return result;
    }

    MetadataValueType   decode( boost::string_ref& stringRef );

    // expecting: "l<bencoded values>e"
    // The initial l and trailing e are beginning and ending delimiters. Lists may contain any bencoded type, including integers, strings, dictionaries, and even lists within other lists.
    std::vector< MetadataValueType > decode_list( boost::string_ref& stringRef )
    {
        assert( !stringRef.empty() && stringRef.front() == 'l' );
        stringRef = stringRef.substr( 1, stringRef.size() - 1 ); // remove 'l'

        std::vector< MetadataValueType > result; // TODO: pseudo pre-allocation ?
        while ( stringRef.front() != 'e' )
            result.emplace_back( decode( stringRef ) );
        return result;
    }

    std::unordered_map< std::string, MetadataValueType > decode_dictionary( boost::string_ref& stringRef )
    {
        assert( !stringRef.empty() && stringRef.front() == 'd' );
        stringRef = stringRef.substr( 1, stringRef.size() - 1 ); // remove 'd'

        std::unordered_map< std::string, MetadataValueType > result;
        while ( stringRef.front() != 'e' )
        {
            if ( '0' >= stringRef.front() || stringRef.front() >= '9' )
                throw std::invalid_argument( "Invalid dictionary key" );

            auto key = decode_string( stringRef );
            result[ key ] = decode( stringRef );
        }
        return result;
    }

    //
    MetadataValueType   decode( boost::string_ref& stringRef )
    {
        // string are the most frequent
        if ( '0' <= stringRef.front() && stringRef.front() <= '9' )
            return decode_string( stringRef );

        if ( stringRef.front() == 'i' )
            return decode_naive_integer( stringRef );

        if ( stringRef.front() == 'l' )
            return decode_list( stringRef );

        if ( stringRef.front() == 'd' )
            return decode_dictionary( stringRef );

        throw std::invalid_argument( "Invalid front token: " + stringRef.front() );
    }

    MetadataValueType   decode( const std::string& metadataContent )
    {
        return decode( boost::string_ref( metadataContent ) );
    }
}

BOOST_AUTO_TEST_CASE( DecodeStringTest )
{
    BOOST_CHECK_THROW( decode( "5e:salut" ), std::invalid_argument );
    BOOST_CHECK_THROW( decode( "6:salut" ), std::invalid_argument );

    BOOST_CHECK( boost::get< std::string >( decode( "5:salut" ) ) == "salut" );
    BOOST_CHECK( boost::get< std::string >( decode( "0:" ) ) == "" );
}

BOOST_AUTO_TEST_CASE( DecodeIntegerTest )
{
    BOOST_CHECK_THROW( decode( "ie" ), std::invalid_argument );
    BOOST_CHECK_THROW( decode( "i-+e" ), std::invalid_argument );
    BOOST_CHECK_THROW( decode( "i5" ), std::invalid_argument );
    BOOST_CHECK_THROW( decode( "i5o6e" ), std::invalid_argument );

    BOOST_CHECK( boost::get< long long >( decode( "i5e" ) ) == 5 );
    BOOST_CHECK( boost::get< long long >( decode( "i-++--5e" ) ) == -5 );
}

BOOST_AUTO_TEST_CASE( DecodeListTest )
{
    BOOST_CHECK( boost::get< std::vector< MetadataValueType > >( decode( "le" ) ).empty() );
    BOOST_CHECK( boost::get< std::vector< MetadataValueType > >( decode( "li5e5:salute" ) ).size() == 2 );
}

BOOST_AUTO_TEST_CASE( DecodeDictionaryTest )
{
    BOOST_CHECK_THROW( decode( "d3:keye" ), std::invalid_argument );

    using MetadataDictionary = std::unordered_map< std::string, MetadataValueType >;
    BOOST_CHECK( boost::get< MetadataDictionary >( decode( "de" ) ).empty() );
    BOOST_CHECK( boost::get< MetadataDictionary >( decode( "d3:key5:valuee" ) ).size() == 1 );
    BOOST_CHECK( boost::get< MetadataDictionary >( decode( "d3:key5:value4:key2li5e3:supee" ) ).size() == 2 );
}

BOOST_AUTO_TEST_SUITE_END() // ! DecoderTestSuite
