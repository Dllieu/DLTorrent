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
        utility::GenericBigEndianBuffer< 2048 >     buffer_; // must reduce size corresponding of the max size message

        boost::asio::io_service                     ioService_;
        bai::udp::socket                            socket_;

        // should be infos by endpoint
        int                                         transactionId_;
        uint64_t                                    connectionId_;
        // ! should be infos by endpoint

        // should be known at initialization
        std::string                                 peerId_;

    private:
        struct PImpl;
        std::unique_ptr< PImpl >    pimpl_;




        const RootMetaInfo&                          root_;
    };
}

#endif // ! __PARSING_TRACKER_H__
