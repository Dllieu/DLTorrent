//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/uuid/sha1.hpp>

#include <iostream>

#include "parsing/BEncoder.h"
#include "parsing/TorrentReader.h"
#include "parsing/RootMetaInfo.h"

#include "utility/RandomGenerator.h"
#include "utility/GenericBigEndianBuffer.h"
#include "utility/Sha1Encoder.h"

using namespace parsing;
using namespace boost::asio::ip;

BOOST_AUTO_TEST_SUITE( TrackerTestSuite )
void display( char* hash )
{
    std::cout << "SHA1: " << std::hex;
    for ( int i = 0; i < 20; ++i )
    {
        std::cout << ( ( hash[ i ] & 0x000000F0 ) >> 4 )
            << ( hash[ i ] & 0x0000000F );
    }
    std::cout << std::endl; // Das wars  
}
// recup https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/net/protocol_buffer.h
// set baser sur l'implem https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/tracker/tracker_udp.cc
// https://github.com/rakshasa/libtorrent/tree/master/src/tracker
BOOST_AUTO_TEST_CASE( TrackerTest )
{
    auto rootMetaInfo = TorrentReader::read( "E:\\Downloads\\example.torrent" );
    //auto rootMetaInfo = TorrentReader::read( "E:\\Downloads\\hashinfo_c9c6645661a6a7311c0aa5e2e22d7f70a58e12e7.torrent" );

    std::cout << rootMetaInfo.root_ << std::endl;

    // tracker : can be either http (TCP) (with get request) or UDP -> request it -> it will answer with bencoded answer
    //Tracker tracker( boost::get< std::string >( rootDictionary[ "announce" ] ) );

    // UDP tracker
    // http://www.bittorrent.org/beps/bep_0015.html

    // http://code.openhub.net/file?fid=0U90FtOiCf5VWmcp1aR3WJj0LN8&cid=AD3GsYPg8AU&s=&fp=395380&projSelected=true&fp=395380&projSelected=true#L0
    // example : http://stackoverflow.com/questions/22791021/bittorrent-client-getting-peer-list-from-trackers-python

    utility::GenericBigEndianBuffer< 2048 > buffer;


    boost::asio::io_service io_service;

    //socket.set_option(  );

    //udp://tracker.publicbt.com:80/announce
    //udp://tracker.openbittorrent.com:80/announce
    //udp://tracker.istole.it:80/announce

    std::vector< std::string > hostnames {
        "tracker.publicbt.com",
        "tracker.openbittorrent.com",
    };
    auto trackerHostname = hostnames[0];
    std::cout << "Hostname: " << trackerHostname << std::endl;

    udp::resolver resolver( io_service );
    udp::resolver::query query( udp::v4(), trackerHostname, "80" ); // ip.dst == 179.43.146.110 || ip.src == 179.43.146.110
    udp::endpoint endpoint = *resolver.resolve( query ); // can verif si it != end

    std::cout << "endpoint used: " << endpoint << std::endl;

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

    auto connectionId = static_cast< uint64_t >( 0x41727101980 ); // not written correctly for some reason, wireshark paked should look like "udp_tracker_connection"
    auto action = 0; // connect
    auto transactionId = utility::RandomGenerator< int >::instance().generate();

    // PREPARE FIRST HANDSHAKE DATA
    std::cout << "transactionId generated: " << transactionId << std::endl;
    buffer << connectionId << action << transactionId;
    socket.send_to( boost::asio::buffer( buffer.getDataForReading(), buffer.size() ), endpoint );

    // RECEIVE CONNECTION_ID
    socket.receive_from( boost::asio::buffer( buffer.getDataForWriting( 16 ), 16 ), endpoint );
    buffer >> action >> transactionId >> connectionId;
    std::cout << "received connection id: " << action << "|" << transactionId << "|" << connectionId << std::endl;
    buffer.clear(); // useless

    // SEND ANNOUNCE REQUEST
    // Offset  Size    Name    Value
    //  0       64 - bit integer  connection_id
    //  8       32 - bit integer  action          1 // announce
    //  12      32 - bit integer  transaction_id
    //  16      20 - byte string  info_hash
    //  36      20 - byte string  peer_id
    //  56      64 - bit integer  downloaded
    //  64      64 - bit integer  left
    //  72      64 - bit integer  uploaded
    //  80      32 - bit integer  event           0 // 0: none; 1: completed; 2: started; 3: stopped
    //  84      32 - bit integer  IP address      0 // default
    //  88      32 - bit integer  key
    //  92      32 - bit integer  num_want - 1 // default
    //  96      16 - bit integer  port
    
    action = 1;
    buffer << connectionId << action << transactionId;

    std::string toEncode = BEncoder::encode( rootMetaInfo.root_[ "info" ] );
    auto info_hash = utility::Sha1Encoder::instance().encode( toEncode );
    std::cout << "SHA1: " << utility::sha1_to_string( info_hash ) << std::endl;

    buffer.writeArray( info_hash );



    /*udp::endpoint endpoint( "open.demonii.com", 1337 );
    std::cout << endpoint << std::endl;
    socket.connect( endpoint );*/
}

// ip.dst == 188.226.220.190 || ip.src == 188.226.220.190 || ip.dst == 31.172.63.225 || ip.src == 31.172.63.225 || ip.dst == 207.244.94.46 || ip.src == 207.244.94.46

BOOST_AUTO_TEST_SUITE_END() // ! TrackerTestSuite
