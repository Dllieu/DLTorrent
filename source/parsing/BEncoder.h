//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_BENCODER_H__
#define __PARSING_BENCODER_H__

#include "MetaInfo.h"

namespace parsing
{
    class BEncoder
    {
    public:
        static std::string  encode( const MetaInfo& metainfo );
    };
}

#endif // ! __PARSING_BENCODER_H__