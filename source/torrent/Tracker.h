//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_TRACKER_H__
#define __TORRENT_TRACKER_H__

#include "RootMetaInfo.h"

namespace torrent
{
    class Tracker
    {
    public:
        Tracker( Tracker&& tracker );
        ~Tracker();

        Tracker&    operator=( Tracker&& tracker );

        const RootMetaInfo&     getRootMetaInfo() const;
        void                    start();

    private:
        friend class TorrentReader;
        explicit Tracker( RootMetaInfo&& root );

    private:
        struct PImpl;
        std::unique_ptr< PImpl >    pimpl_;
    };
}

#endif // ! __TORRENT_TRACKER_H__
