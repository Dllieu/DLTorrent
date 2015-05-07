//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_ROOTMETAINFODICTIONARY_H__
#define __PARSING_ROOTMETAINFODICTIONARY_H__

#include "MetaInfo.h"

namespace parsing
{
    class RootMetaInfo
    {
    public:
        RootMetaInfo( const RootMetaInfo& ) = delete;
        RootMetaInfo& operator=( const RootMetaInfo& ) = delete;

    private:
        friend class TorrentReader;
        explicit RootMetaInfo( MetaInfoDictionary&& root );

    public:
        MetaInfoDictionary    root_;
    };
}

#endif // ! __PARSING_ROOTMETAINFODICTIONARY_H__
