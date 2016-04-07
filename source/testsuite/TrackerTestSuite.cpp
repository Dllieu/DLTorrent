//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio.hpp>
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/use_future.hpp>
#pragma warning( pop )

#include <boost/utility/string_ref.hpp>
#include <boost/uuid/sha1.hpp>

#include <thread>
#include <iostream>

#include "torrent/TorrentReader.h"
#include "torrent/Tracker.h"
#include "torrent/Torrent.h"
#include "torrent/Peer.h"

#include "utility/IoService.h"
#include "utility/DebugTools.h"
#include "utility/Conversion.h"

using namespace torrent;
using namespace boost::asio::ip;

BOOST_AUTO_TEST_SUITE( TrackerTestSuite )

// set baser sur l'implem https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/tracker/tracker_udp.cc
// https://github.com/rakshasa/libtorrent/tree/master/src/tracker

// tracker : can be either http (TCP) (with get request) or UDP -> request it -> it will answer with bencoded answer

// UDP tracker
// http://www.bittorrent.org/beps/bep_0015.html

// http://code.openhub.net/file?fid=0U90FtOiCf5VWmcp1aR3WJj0LN8&cid=AD3GsYPg8AU&s=&fp=395380&projSelected=true&fp=395380&projSelected=true#L0
// example : http://stackoverflow.com/questions/22791021/bittorrent-client-getting-peer-list-from-trackers-python

#include <boost/dynamic_bitset.hpp>

BOOST_AUTO_TEST_CASE( TrackerTest )
{
    std::atomic< bool > mustStop( false );
    std::thread thread( [ &mustStop ]
    {
        boost::asio::io_service::work work( utility::IoService::instance() );

        while ( !mustStop )
        {
            std::cout << "-> utility::IoService::instance().run()" << std::endl;
            utility::IoService::instance().run();
        }
    } );

    auto torrent = TorrentReader::read( "E:\\Downloads\\example.torrent" );
    Tracker tracker;

    // Copy as we need non const...
    auto peerEndPoints = tracker.peerEndpoints( torrent );
    auto wiresharkDebug = utility::generate_wireshark_filter( peerEndPoints );

    Peer peer( peerEndPoints, torrent.getHashInfo(), torrent.getPieceInfo() );
    peer.connect();

    mustStop = true;
    utility::IoService::instance().stop();
    thread.join();
}

BOOST_AUTO_TEST_SUITE_END() // ! TrackerTestSuite
