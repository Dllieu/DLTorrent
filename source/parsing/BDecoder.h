//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_BDECODER_H__
#define __PARSING_BDECODER_H__

#include "MetaInfo.h"

namespace parsing
{
    class BDecoder
    {
    public:
        static MetaInfo decode( const std::string& encodedMetaInfo );
        static MetaInfo decode( const char* encodedMetaInfo, size_t size )
        {
            return decode( std::string( encodedMetaInfo, encodedMetaInfo + size ) );
        }

        static MetaInfo decodeNonBinary( const std::string& encodedMetaInfo )
        {
            return decode( encodedMetaInfo );
        }

    private:
        // Force decode to be called with an explicit type
        // prevent implicit conversion from const char* -> string :
        //  - encodedMetaInfo usually contains non printable character (e.g. can contains several '\0', the implicit copy will stop on the first '\0')
        template < typename T >
        static MetaInfo decode( const T& encodedMetaInfo );
    };
}

#endif // ! __PARSING_BDECODER_H__
