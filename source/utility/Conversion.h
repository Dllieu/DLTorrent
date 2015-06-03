//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_CONVERSION_H__
#define __TORRENT_CONVERSION_H__

#include <boost/utility/string_ref.hpp>
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
}

#undef FORMAT_EXCEPTION_MESSAGE

#endif // ! __TORRENT_CONVERSION_H__
