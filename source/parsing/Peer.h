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
