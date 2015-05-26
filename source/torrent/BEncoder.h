//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_BENCODER_H__
#define __TORRENT_BENCODER_H__

#include "MetaInfo.h"

namespace torrent
{
    class BEncoder
    {
    public:
        static std::string  encode( const MetaInfo& metainfo );
    };
}

#endif // ! __TORRENT_BENCODER_H__
