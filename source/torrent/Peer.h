//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

#include <boost/asio/ip/tcp.hpp>

#include <vector>
#include <array>
#include <memory>

// TODO : when creating the peer, give him a valid socket, peer should only have the message communication
// see https://github.com/mpetazzoni/ttorrent/blob/master/core/src/main/java/com/turn/ttorrent/client/peer/SharingPeer.java
namespace torrent
{
    class PieceInfo;

    // Should be PeerManager (worker thread on peers)
    // Should not use pimpl as there's frequent new Peer?
    class Peer
    {
    public:
        Peer( boost::asio::ip::tcp::endpoint& endpoint, const std::array< char, 20 >& hashInfo, const PieceInfo& piece );
        // Forward declaration of Pimpl
        ~Peer();

        Peer::Peer( Peer&& );
        Peer& Peer::operator=( Peer&& );

        void    start();

    private:
        struct PImpl;
        std::unique_ptr< PImpl >    pimpl_;
    };
}
