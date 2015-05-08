//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <boost/utility/string_ref.hpp>

#include <iostream>

#include "parsing/TorrentReader.h"
#include "parsing/RootMetaInfo.h"
#include "utility/RandomGenerator.h"
#include "utility/GenericBigEndianBuffer.h"

using namespace parsing;
using namespace boost::asio::ip;

BOOST_AUTO_TEST_SUITE( TrackerTestSuite )

// recup https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/net/protocol_buffer.h
// set baser sur l'implem https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/tracker/tracker_udp.cc
// https://github.com/rakshasa/libtorrent/tree/master/src/tracker
BOOST_AUTO_TEST_CASE( TrackerTest )
{
    auto rootMetaInfo = TorrentReader::read( "E:\\Downloads\\example.torrent" );
    auto trackerUdpAddress = boost::get< std::string >( rootMetaInfo.root_[ "announce" ] );

    //unsigned long long ull = 0x41727101980;
    //std::cout << ull << std::endl;
    //ull ^= unsigned long long(-1);
    //std::cout << ull << std::endl;
    //ull ^= unsigned long long( -1 );
    //std::cout << ull << std::endl;

    // tracker : can be either http (TCP) (with get request) or UDP -> request it -> it will answer with bencoded answer
    //Tracker tracker( boost::get< std::string >( rootDictionary[ "announce" ] ) );

    // UDP tracker
    // http://www.bittorrent.org/beps/bep_0015.html

    // http://code.openhub.net/file?fid=0U90FtOiCf5VWmcp1aR3WJj0LN8&cid=AD3GsYPg8AU&s=&fp=395380&projSelected=true&fp=395380&projSelected=true#L0
    // example : http://stackoverflow.com/questions/22791021/bittorrent-client-getting-peer-list-from-trackers-python
    std::cout << trackerUdpAddress << std::endl;

    utility::GenericBigEndianBuffer< 2048 > buffer;

    // Prepare first handshake data
    {
        auto connectionId = static_cast< uint64_t >( 0x41727101980 ); // not written correctly for some reason, wireshark paked should look like "udp_tracker_connection"
        auto action = 0; // connect
        auto transactionId = utility::RandomGenerator< int >::instance().generate();

        std::cout << "transactionId generated: " << transactionId <<std::endl;
        buffer << connectionId << action << transactionId;
    }


    boost::asio::io_service io_service;

    //socket.set_option(  );

    //udp://tracker.publicbt.com:80/announce
    //udp://tracker.openbittorrent.com:80/announce
    //udp://tracker.istole.it:80/announce

    udp::resolver resolver( io_service );
    udp::resolver::query query( udp::v4(), "tracker.openbittorrent.com", "80" ); // ip.dst == 179.43.146.110 || ip.src == 179.43.146.110
    udp::endpoint endpoint = *resolver.resolve( query ); // can verif si it != end

    std::cout << endpoint << std::endl;

    udp::socket socket( io_service );
    socket.open( udp::v4() );
    // deja fiat
    //socket.open( udp::v4() );
    socket.connect( endpoint );

    // http://stackoverflow.com/questions/18853149/how-to-combine-three-variables-to-send-using-boost-asio
    // What endian does the bittorrent protocol use ?
    // It's big endian, so any solution here that relies on casting won't work on your typical consumer electronics these days, because these use little - endian format in memory.
    // In creating your buffer to send, you therefore also have to swap the bytes.


    // todo string -> endpoint (then diff tcp / udp)
    socket.send_to( boost::asio::buffer( buffer.getDataForReading(), buffer.size() ), endpoint );
    buffer.clear();

    // RECEIVE CONNECTION_ID
    socket.receive_from( boost::asio::buffer( buffer.getDataForWriting( 16 ), 16 ), endpoint );
    {
        int action = 0;
        int transactionId = 0;
        uint64_t connectionId = 0;

        buffer >> action >> transactionId >> connectionId;

        std::cout << "received connection id: " << action << "|" << transactionId << "|" << connectionId << std::endl;
    }
    buffer.clear();




    /*udp::endpoint endpoint( "open.demonii.com", 1337 );
    std::cout << endpoint << std::endl;
    socket.connect( endpoint );*/
}

BOOST_AUTO_TEST_SUITE_END() // ! TrackerTestSuite
