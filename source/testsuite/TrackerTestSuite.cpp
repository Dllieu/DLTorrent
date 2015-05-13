//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/uuid/sha1.hpp>

#include <thread>
#include <iostream>

#include "parsing/TorrentReader.h"
#include "parsing/Tracker.h"
#include "parsing/RootMetaInfo.h"

#include "utility/DebugTools.h"

using namespace parsing;
using namespace boost::asio::ip;

BOOST_AUTO_TEST_SUITE( TrackerTestSuite )

// set baser sur l'implem https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/tracker/tracker_udp.cc
// https://github.com/rakshasa/libtorrent/tree/master/src/tracker

// tracker : can be either http (TCP) (with get request) or UDP -> request it -> it will answer with bencoded answer

// UDP tracker
// http://www.bittorrent.org/beps/bep_0015.html

// http://code.openhub.net/file?fid=0U90FtOiCf5VWmcp1aR3WJj0LN8&cid=AD3GsYPg8AU&s=&fp=395380&projSelected=true&fp=395380&projSelected=true#L0
// example : http://stackoverflow.com/questions/22791021/bittorrent-client-getting-peer-list-from-trackers-python
BOOST_AUTO_TEST_CASE( TrackerTest )
{
    auto tracker = TorrentReader::read( "E:\\Downloads\\example.torrent" );

    // DEBUG Utils
    std::string wiresharkFilter = utility::generate_wireshark_filter( tracker.getRootMetaInfo().getAnnouncers() );
    // ! DEBUG Utils

    tracker.getRootMetaInfo().display();
    tracker.scrape();

    for (;;)
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
}

BOOST_AUTO_TEST_SUITE_END() // ! TrackerTestSuite
