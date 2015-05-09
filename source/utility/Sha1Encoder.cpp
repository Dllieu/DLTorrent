//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include "Sha1Encoder.h"

using namespace utility;

std::array< char, 20 >     Sha1Encoder::encode( const std::string& toEncode )
{
    sha1_.process_bytes( toEncode.c_str(), toEncode.size() );
    sha1_.get_digest( digest_ );

    std::array< char, 20 > result;
    for ( auto i = 0; i < 5; ++i )
    {
        const char* tmp = reinterpret_cast< char* >( digest_ );

        result[ i * 4 ] = tmp[ i * 4 + 3 ];
        result[ i * 4 + 1 ] = tmp[ i * 4 + 2 ];
        result[ i * 4 + 2 ] = tmp[ i * 4 + 1 ];
        result[ i * 4 + 3 ] = tmp[ i * 4 ];
    }
    return result;
}

std::string     utility::sha1_to_string( const std::array< char, 20 >& hash )
{
    static const char alphabet[] = "0123456789abcdef";

    std::string result( 40, '\0' );

    auto i = static_cast< size_t >( 0 );
    for ( auto c : hash )
    {
        result[ i++ ] = alphabet[ ( c & 0x000000F0 ) >> 4 ];
        result[ i++ ] = alphabet[ c & 0x0000000F ];
    }
    return result;
}
