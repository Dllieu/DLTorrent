//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PEER_TRACKER_H__
#define __PEER_TRACKER_H__

#include <boost/asio/ip/tcp.hpp>

#include <vector>
#include <array>
#include <memory>

namespace bai = boost::asio::ip;

// TODO : when creating the peer, give him a valid socket, peer should only have the message communication
// see https://github.com/mpetazzoni/ttorrent/blob/master/core/src/main/java/com/turn/ttorrent/client/peer/SharingPeer.java

namespace parsing
{
    class Peer
    {
    public:
        Peer( const std::vector< bai::tcp::endpoint >& endpoints, const std::array< char, 20 >& hashInfo );
        // Forward declaration of Pimpl
        ~Peer();

        void    connect();

    private:
        struct PImpl;
        std::unique_ptr< PImpl >    pimpl_;
    };
}

#endif // ! __PEER_TRACKER_H__
