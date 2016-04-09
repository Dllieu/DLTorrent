#include "PeerManager.h"

#include <iostream>

#include "Torrent.h"
#include "Peer.h"

using namespace torrent;

void    PeerManager::downloadTorrent( std::vector< boost::asio::ip::tcp::endpoint >& endpoints, const Torrent& torrent )
{
    std::cout << "Attempt to connect to " << endpoints.size() << " endpoints" << std::endl;
    for ( auto& endpoint : endpoints )
    {
        Peer peer( endpoint, torrent.getHashInfo(), torrent.getPieceInfo() );
        peer.start();
    }
}
