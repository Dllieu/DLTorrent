//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <iostream>

#include "parsing/TorrentReader.h"
#include "parsing/RootMetaInfo.h"

using namespace parsing;

BOOST_AUTO_TEST_SUITE( TrackerTestSuite )

BOOST_AUTO_TEST_CASE( TrackerTest )
{
    auto rootMetaInfo = TorrentReader::read( "E:\\Downloads\\example.torrent" );
    std::cout << boost::get< std::string >( rootMetaInfo.root_[ "announce" ] ) << std::endl;
    // tracker : can be either http (TCP) (with get request) or UDP -> request it -> it will answer with bencoded answer
    //Tracker tracker( boost::get< std::string >( rootDictionary[ "announce" ] ) );
}

BOOST_AUTO_TEST_SUITE_END() // ! TrackerTestSuite
