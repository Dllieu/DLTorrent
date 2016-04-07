#pragma once

#include "Torrent.h"

namespace torrent
{
    /*
     * @name: TorrentManager
     * @brief: Manage the torrent(s ?) underlying content download (/ upload later on)
     *       - Track the peer
     *       - Attempt to connect to the peer
     *       - The peer will then call the "MessageHandler"
     */
    class TorrentManager
    {
    public:
        TorrentManager() = default;

        void    addTorrent( Torrent&& torrent );

    private:
        //std::vector< Torrent >     torrent_;
    };
}
