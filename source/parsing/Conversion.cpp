//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <exception>

#include "Conversion.h"

unsigned int    parsing::naive_uint_conversion( const boost::string_ref& stringRef )
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

long long   parsing::naive_ll_conversion( const boost::string_ref& stringRef )
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
    if ( !finishReadSign )
        throw std::invalid_argument( std::string( __FUNCTION__ ) + "with " + std::string( stringRef ) );

    return ( isNegative ? -1 : 1 ) * result;
}
