//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_TRACKER_H__
#define __TORRENT_TRACKER_H__

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/ip/tcp.hpp>
#pragma warning( pop )

namespace torrent
{
    class Torrent;

    // TODO: Announcer should be able to handle multiple torrent (i.e. for scrapping)
    //       - here is a naive approache 1 announcer <-> 1 torrent (can't benefit from scrapping)
    class Tracker
    {
    public:
        explicit Tracker();
        ~Tracker();

        Tracker( const Tracker & ) = delete;
        Tracker&    operator=( const Tracker & ) = delete;

        const std::vector< boost::asio::ip::tcp::endpoint >&        peerEndpoints( const Torrent& torrent );

    private:
        struct PImpl;
        std::unique_ptr< PImpl >    pimpl_;
    };
}

#endif // ! __TORRENT_TRACKER_H__
