#pragma once

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/ip/udp.hpp>
#pragma warning( pop )

#include "utility/GenericBigEndianBuffer.h"

namespace torrent
{
    struct TrackerSocket
    {
        using BufferType = utility::GenericBigEndianBuffer< 2048 >;

        TrackerSocket( boost::asio::ip::udp::endpoint& endpoint );
        ~TrackerSocket() = default;

        BufferType&     buffer()
        {
            return buffer_;
        }

        short   port() const
        {
            return socket_.remote_endpoint().port();
        }

        bool    connect();
        bool    send();
        bool    receive();

    private:
        boost::asio::ip::udp::socket        socket_;
        boost::asio::ip::udp::endpoint&     endpoint_;

        BufferType                          buffer_;
    };
}
