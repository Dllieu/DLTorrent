#pragma once

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/ip/tcp.hpp>
#pragma warning( pop )

#include "utility/GenericBigEndianBuffer.h"

namespace torrent
{
    struct PeerSocket
    {
        // TODO: check for the good size
        using ReceiveBufferType = utility::GenericBigEndianBuffer< 32 * 1024 >; // huge to handle a chunk of piece
        using SendBufferType = utility::GenericBigEndianBuffer< 2048 >;

        PeerSocket( boost::asio::ip::tcp::endpoint& endpoint, int timeout );
        ~PeerSocket() = default;

        SendBufferType&     sendBuffer()
        {
            return sendBuffer_;
        }

        ReceiveBufferType&  receiveBuffer()
        {
            return receiveBuffer_;
        }

        short   port() const
        {
            return socket_.remote_endpoint().port();
        }

        void    setTimeout( int timeout )
        {
            timeout_ = timeout;
        }

        bool    connect();
        bool    send();
        bool    receive();
        bool    waitReceiveUntruncatedMessage();

    private:
        boost::asio::ip::tcp::socket        socket_;
        boost::asio::ip::tcp::endpoint&     endpoint_;
        int                                 timeout_;

        ReceiveBufferType                   receiveBuffer_;
        SendBufferType                      sendBuffer_;
    };
}
