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

using namespace parsing;
using namespace boost::asio::ip;

BOOST_AUTO_TEST_SUITE( TrackerTestSuite )

namespace
{
    struct POD_udp_tracker_connect
    {
        unsigned long long  connection_id;
        int                 action;
        int                 transaction_id;
    };

    struct POD_udp_tracker_connect_answer
    {
        int                 action;
        int                 transaction_id;
        unsigned long long  connection_id;
    };
}

// recup https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/net/protocol_buffer.h
// set baser sur l'implem https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/tracker/tracker_udp.cc

// https://github.com/rakshasa/libtorrent/tree/master/src/tracker
BOOST_AUTO_TEST_CASE( TrackerTest )
{
    auto rootMetaInfo = TorrentReader::read( "E:\\Downloads\\example.torrent" );
    auto trackerUdpAddress = boost::get< std::string >( rootMetaInfo.root_[ "announce" ] );


    // tracker : can be either http (TCP) (with get request) or UDP -> request it -> it will answer with bencoded answer
    //Tracker tracker( boost::get< std::string >( rootDictionary[ "announce" ] ) );

    // UDP tracker
    // http://www.bittorrent.org/beps/bep_0015.html

    // example : http://stackoverflow.com/questions/22791021/bittorrent-client-getting-peer-list-from-trackers-python
    std::cout << trackerUdpAddress << std::endl;

    POD_udp_tracker_connect handShake;

    handShake.connection_id = 0x41727101980; // default connection Id
    handShake.action = 0; // connect
    handShake.transaction_id = utility::RandomGenerator< int >::instance().generate();


    boost::asio::io_service io_service;

    //socket.set_option(  );

    //udp://tracker.publicbt.com:80/announce
    //udp://tracker.openbittorrent.com:80/announce
    //udp://tracker.istole.it:80/announce

    udp::resolver resolver( io_service );
    udp::resolver::query query( udp::v4(), "tracker.openbittorrent.com", "80" );
    udp::endpoint endpoint = *resolver.resolve( query ); // can verif si it != end

    std::cout << endpoint << std::endl;

    udp::socket socket( io_service );

    std::cout << endpoint.protocol().family() << AF_INET6 << std::endl;
    std::cout << endpoint.protocol().type() << std::endl;
    std::cout << endpoint.protocol().protocol() << std::endl;
    socket.open( endpoint.protocol() );
    // deja fiat
    //socket.open( udp::v4() );
    socket.connect( endpoint );

    // todo string -> endpoint (then diff tcp / udp)
    socket.send_to( boost::asio::buffer( &handShake, sizeof( handShake ) ), endpoint );

    //POD_udp_tracker_connect_answer answer;
    std::array< char, 2048 > buffer;
    socket.receive_from( boost::asio::buffer( buffer ), endpoint );


    std::cout << "received connection id: " << buffer.data() << std::endl;

    /*udp::endpoint endpoint( "open.demonii.com", 1337 );
    std::cout << endpoint << std::endl;
    socket.connect( endpoint );*/
}

BOOST_AUTO_TEST_SUITE_END() // ! TrackerTestSuite
