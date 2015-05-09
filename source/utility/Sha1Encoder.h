//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __UTILITY_SHA1ENCODER_H__
#define __UTILITY_SHA1ENCODER_H__

#include <boost/uuid/sha1.hpp>
#include <array>

#include "ThreadSafeSingleton.h"

namespace utility
{
    class Sha1Encoder : public ThreadSafeSingleton< Sha1Encoder >
    {
    public:
        std::array< char, 20 >     encode( const std::string& toEncode );

    private:
        boost::uuids::detail::sha1  sha1_;
        unsigned int                digest_[ 5 ];
    };

    std::string     sha1_to_string( const std::array< char, 20 >& hash );
}

#endif // ! __UTILITY_SHA1ENCODER_H__
