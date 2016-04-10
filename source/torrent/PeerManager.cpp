#include "PeerManager.h"

#include "utility/Logger.h"

#include "Torrent.h"
#include "Peer.h"

using namespace torrent;

void    PeerManager::downloadTorrent( std::vector< boost::asio::ip::tcp::endpoint >& endpoints, const Torrent& torrent )
{
    LOG_INFO << "Attempt to connect to " << endpoints.size() << " endpoints";
    for ( auto& endpoint : endpoints )
    {
        Peer peer( endpoint, torrent.getHashInfo(), torrent.getPieceInfo() );
        peer.start();
    }
}
