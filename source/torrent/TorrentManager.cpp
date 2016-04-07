#include "TorrentManager.h"
#include "Tracker.h"

using namespace torrent;

void    TorrentManager::addTorrent( Torrent&& torrentrv )
{
    // TODO: persist torrent added
    // i.e. add it on some containers if it doesn't exist yet, otherwise warn user (throw? or just return bool)
    auto torrent = std::move( torrentrv );

    // 1 - recup list peers de tracker si y'en a aucunes

}
