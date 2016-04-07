//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_CONVERSION_H__
#define __TORRENT_CONVERSION_H__

#include <boost/utility/string_ref.hpp>
#include <boost/dynamic_bitset.hpp>
#include <exception>

#define FORMAT_EXCEPTION_MESSAGE( MESSAGE ) \
    std::string( __FUNCTION__ ) + "with " + std::string( MESSAGE )

namespace utility
{
    // cheap str_to_numeric as bencoding numeric are easy to parse (no exponential, only power of 10, no digit separator, etc...)
    inline unsigned int    naive_uint_conversion( const boost::string_ref& stringRef )
    {
        unsigned int result = 0;
        for ( auto c : stringRef )
        {
            if ( c < '0' || c > '9' )
                throw std::invalid_argument( FORMAT_EXCEPTION_MESSAGE( stringRef ) );

            result = result * 10 + c - '0';
        }
        return result;
    }

    inline long long   naive_ll_conversion( const boost::string_ref& stringRef )
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
                throw std::invalid_argument( FORMAT_EXCEPTION_MESSAGE( stringRef ) );

            result = result * 10 + c - '0';
        }
        if ( !finishReadSign )
            throw std::invalid_argument( FORMAT_EXCEPTION_MESSAGE( stringRef ) );

        return ( isNegative ? -1 : 1 ) * result;
    }

    // Extra bit set to 0
    inline char    bits_to_char( const boost::dynamic_bitset<>& bitset, size_t index )
    {
        char result = 0;
        size_t offset = 0;
        for ( ; offset < 8 && index + offset < bitset.size(); ++offset )
            result |= static_cast< char >( bitset[ index + offset ] ) << ( 7 - offset );

        return result;
    }

    inline std::vector< char >   bitset_to_bytes( const boost::dynamic_bitset<>& bitset )
    {
        std::vector< char > result;
        result.reserve( bitset.size() / 8 + ( bitset.size() % 8 != 0 ? 1 : 0 ) );

        for ( auto i = 0; i < bitset.size(); i += 8 )
        {
            char byte = 0;
            size_t offset = 0;
            for ( ; offset < 8 && i + offset < bitset.size(); ++offset )
                byte |= static_cast< char >( bitset[ i + offset ] ) << ( 7 - offset );

            result.emplace_back( byte );
        }
        return result;
    }

    /*
    * @params bitsetSize : exact number of bits (i.e. torrent have bits padding for the last byte)
    */
    inline boost::dynamic_bitset<>  bytes_to_bitset( const std::vector< char >& bytes, size_t bitsetSize )
    {
        boost::dynamic_bitset<> result( bitsetSize );

        for ( auto i = 0; i < result.size() && i / 8 < bytes.size(); i += 8 )
        {
            auto c = bytes[ i / 8 ];
            for ( auto j = 0; j < 8 && i + j < result.size(); ++j )
                result.set( i + j, ( c >> ( 7 - j ) ) & 1 );
        }
        return result;
    }
}

#undef FORMAT_EXCEPTION_MESSAGE

#endif // ! __TORRENT_CONVERSION_H__
