//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PEER_TRACKER_H__
#define __PEER_TRACKER_H__

#include <boost/asio/ip/tcp.hpp>

namespace bai = boost::asio::ip;

namespace parsing
{
    class Peer
    {
    public:
        Peer( const std::vector< bai::tcp::endpoint >& endpoints );

        void    connect();

    private:
        void    onConnect( const boost::system::error_code& errorCode, const bai::tcp::endpoint& endpoint );
        void    checkDeadline( const boost::system::error_code& errorCode );

    private:
        bai::tcp::socket                    socket_;
        boost::asio::deadline_timer         deadline_;
        std::vector< bai::tcp::endpoint >   endpoints_;
    };
}

#endif // ! __PEER_TRACKER_H__
