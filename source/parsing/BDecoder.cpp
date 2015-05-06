//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/utility/string_ref.hpp>

#include "BDecoder.h"
#include "CustomException.h"
#include "Conversion.h"

using namespace parsing;

namespace
{
    Metadata   bdecode( boost::string_ref& stringRef );
}

/*static*/ Metadata    BDecoder::decode( const std::string& encodedMetadata )
{
    // Passing by boost::string_ref to benefit from cheap substr
    // Use std::string_view once it's available
    return ::bdecode( boost::string_ref( encodedMetadata ) );
}

namespace
{
    // expecting: "ine"
    // The initial i and trailing e are beginning and ending delimiters. You can have negative numbers such as i-3e.
    // Only the significant digits should be used, one cannot pad the Integer with zeroes. such as i04e. However, i0e is valid
    // handle n as a signed 64bit integer is mandatory to handle "large files" aka .torrent for more that 4Gbyte
    long long     bdecode_naive_integer( boost::string_ref& stringRef )
    {
        if ( stringRef.empty() || stringRef.front() != 'i' )
            throw invalid_metadata_integer( "Can't decode" );

        auto endIndex = stringRef.find_first_of( 'e' );
        if ( endIndex == std::string::npos )
            throw invalid_metadata_integer( "Missing 'e'" );

        if ( endIndex == 1 )
            throw invalid_metadata_integer( "Empty integer" );

        auto result = naive_ll_conversion( stringRef.substr( 1, endIndex - 1 ) );
        stringRef = stringRef.substr( endIndex + 1, stringRef.size() - endIndex - 1 );
        return result;
    }

    // expecting: "n:string" with n == string.size()
    std::string     bdecode_string( boost::string_ref& stringRef )
    {
        if ( stringRef.empty() || '0' > stringRef.front() || stringRef.front() > '9' )
            throw invalid_metadata_string( "Can't decode" );

        auto colonIndex = stringRef.find_first_of( ':' );
        if ( colonIndex == std::string::npos )
            throw invalid_metadata_string( "Missing ':'" );

        auto sizeString = naive_uint_conversion( stringRef.substr( 0, colonIndex ) );

        size_t i = colonIndex + 1;
        if ( i + sizeString > stringRef.size() )
            throw invalid_metadata_string( "Out of bound" );

        auto result = std::string( stringRef.substr( i, sizeString ) );
        i += sizeString;
        stringRef = stringRef.substr( i, stringRef.size() - i );
        return result;
    }

    // expecting: "l<bencoded values>e"
    // The initial l and trailing e are beginning and ending delimiters. Lists may contain any bencoded type, including integers, strings, dictionaries, and even lists within other lists.
    std::vector< Metadata > bdecode_list( boost::string_ref& stringRef )
    {
        if ( stringRef.empty() || stringRef.front() != 'l' )
            throw invalid_metadata_list( "Can't decode" );

        stringRef = stringRef.substr( 1, stringRef.size() - 1 ); // remove 'l'

        std::vector< Metadata > result; // TODO: pseudo pre-allocation ?
        while ( stringRef.front() != 'e' )
            result.emplace_back( bdecode( stringRef ) );

        stringRef = stringRef.substr( 1, stringRef.size() - 1 ); // remove 'e'
        return result;
    }

    std::unordered_map< std::string, Metadata > bdecode_dictionary( boost::string_ref& stringRef )
    {
        if ( stringRef.empty() || stringRef.front() != 'd' )
            throw invalid_metadata_dictionary( "Can't decode" );

        stringRef = stringRef.substr( 1, stringRef.size() - 1 ); // remove 'd'

        std::unordered_map< std::string, Metadata > result;
        while ( stringRef.front() != 'e' )
        {
            auto key = bdecode_string( stringRef );
            result[ key ] = bdecode( stringRef );

            if ( stringRef.empty() )
                throw invalid_metadata_list( "Missing 'e'" );
        }

        stringRef = stringRef.substr( 1, stringRef.size() - 1 ); // remove 'e'
        return result;
    }

    // recursive call
    Metadata   bdecode( boost::string_ref& stringRef )
    {
        if ( stringRef.empty() )
            throw std::invalid_argument( "Can't decode empty string" );

        // string are the most frequent
        if ( '0' <= stringRef.front() && stringRef.front() <= '9' )
            return bdecode_string( stringRef );

        if ( stringRef.front() == 'i' )
            return bdecode_naive_integer( stringRef );

        if ( stringRef.front() == 'l' )
            return bdecode_list( stringRef );

        if ( stringRef.front() == 'd' )
            return bdecode_dictionary( stringRef );

        throw std::invalid_argument( "Invalid front token: " + stringRef.front() );
    }
}
