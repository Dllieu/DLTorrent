//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio.hpp>
#pragma warning( pop )

#include <atomic>
#include <thread>
#include <iostream>

#include "torrent/TorrentReader.h"
#include "torrent/Tracker.h"
#include "torrent/Torrent.h"
#include "torrent/PeerManager.h"

#include "utility/IoService.h"
#include "utility/DebugTools.h"
#include "utility/Logger.h"

using namespace torrent;
using namespace boost::asio::ip;

BOOST_AUTO_TEST_SUITE( PeerTestSuite )

namespace
{
    bool    start_torrent()
    {
        auto torrent = TorrentReader::read( "E:\\Downloads\\example.torrent" );
        Tracker tracker;

        // Copy as we need non const... (TODO: check why)
        auto peerEndPoints = tracker.peerEndpoints( torrent );
        auto wiresharkDebug = utility::generate_wireshark_filter( peerEndPoints );

        PeerManager::downloadTorrent( peerEndPoints, torrent );

        return true;
    }
}

BOOST_AUTO_TEST_CASE( TrackerTest )
{
    utility::Logger::init();

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

    BOOST_CHECK( start_torrent() );

    mustStop = true;
    utility::IoService::instance().stop();
    thread.join();
}

BOOST_AUTO_TEST_SUITE_END() // ! PeerTestSuite
