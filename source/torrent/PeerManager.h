#pragma once

#include <boost/asio/ip/tcp.hpp>

#include <vector>
#include <array>
#include <memory>

namespace torrent
{
    class PieceInfo;
    class Torrent;

    // Todo: handle thread pool (1 thread per peer / shared among torrents)
    class PeerManager
    {
    public:
        static void     downloadTorrent( std::vector< boost::asio::ip::tcp::endpoint >& endpoints, const Torrent& torrent );
    };
}
