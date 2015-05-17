//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_TRACKER_H__
#define __PARSING_TRACKER_H__

#include "RootMetaInfo.h"

namespace parsing
{
    class Tracker
    {
    public:
        const RootMetaInfo&     getRootMetaInfo() const;

        void                    start();

        // Forward declaration of Pimpl
        ~Tracker();

    private:
        friend class TorrentReader;
        explicit Tracker( RootMetaInfo&& root );

    private:
        struct PImpl;
        std::unique_ptr< PImpl >    pimpl_;
    };
}

#endif // ! __PARSING_TRACKER_H__
