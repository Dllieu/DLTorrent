//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_TRACKER_H__
#define __PARSING_TRACKER_H__

#include <boost/asio/ip/udp.hpp>
#include <array>

#include "MetaInfo.h"
#include "RootMetaInfo.h"
#include "utility/GenericBigEndianBuffer.h"

namespace bai = boost::asio::ip;

namespace parsing
{
    class Tracker
    {
    public:
        const RootMetaInfo&     getRootMetaInfo() const;

        void                    scrape();

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
