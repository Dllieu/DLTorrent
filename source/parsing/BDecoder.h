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
    };
}

#endif // ! __PARSING_BDECODER_H__
