//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_BDECODER_H__
#define __PARSING_BDECODER_H__

#include "Metadata.h"

namespace parsing
{
    class BDecoder
    {
    public:
        static Metadata decode( const std::string& encodedMetadata );
        static Metadata decode( const char* encodedMetadata, size_t size )
        {
            return decode( std::string( encodedMetadata, encodedMetadata + size ) );
        }

        static Metadata decodeNonBinary( const std::string& encodedMetadata )
        {
            return decode( encodedMetadata );
        }

    private:
        // Force decode to be called with an explicit type
        // prevent implicit conversion from const char* -> string :
        //  - encodedMetadata usually contains non printable character (e.g. can contains several '\0', the implicit copy will stop on the first '\0')
        template < typename T >
        static Metadata decode( const T& encodedMetadata );
    };
}

#endif // ! __PARSING_BDECODER_H__
