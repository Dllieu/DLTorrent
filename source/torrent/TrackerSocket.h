#pragma once

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/ip/udp.hpp>
#pragma warning( pop )

#include "utility/GenericBigEndianBuffer.h"

namespace bai = boost::asio::ip;

namespace torrent
{
    struct TrackerSocket
    {
        using BufferType = utility::GenericBigEndianBuffer< 2048 >;

        TrackerSocket( bai::udp::endpoint& endpoint );
        ~TrackerSocket();

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
        bai::udp::socket        socket_;
        bai::udp::endpoint&     endpoint_;

        BufferType              buffer_;
    };
}