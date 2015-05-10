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

#include "utility/DebugTools.h"
#include "utility/RandomGenerator.h"
#include "utility/GenericBigEndianBuffer.h"
#include "utility/Sha1Encoder.h"

using namespace parsing;
using namespace boost::asio::ip;

BOOST_AUTO_TEST_SUITE( TrackerTestSuite )

// recup https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/net/protocol_buffer.h
// set baser sur l'implem https://github.com/rakshasa/libtorrent/blob/99e33c005a04c329c32b8bf26c48bd15725dfffd/src/tracker/tracker_udp.cc
// https://github.com/rakshasa/libtorrent/tree/master/src/tracker

// tracker : can be either http (TCP) (with get request) or UDP -> request it -> it will answer with bencoded answer
//Tracker tracker( boost::get< std::string >( rootDictionary[ "announce" ] ) );

// UDP tracker
// http://www.bittorrent.org/beps/bep_0015.html

// http://code.openhub.net/file?fid=0U90FtOiCf5VWmcp1aR3WJj0LN8&cid=AD3GsYPg8AU&s=&fp=395380&projSelected=true&fp=395380&projSelected=true#L0
// example : http://stackoverflow.com/questions/22791021/bittorrent-client-getting-peer-list-from-trackers-python
BOOST_AUTO_TEST_CASE( TrackerTest )
{
    auto rootMetaInfo = TorrentReader::read( "E:\\Downloads\\example.torrent" );
    std::vector< udp::endpoint > endpoints = rootMetaInfo.getAnnouncers();

    // DEBUG Utils
    std::string wiresharkFilter = utility::generate_wireshark_filter( endpoints );
    //std::cout << rootMetaInfo.root_ << std::endl;
    // ! DEBUG Utils

    // Ref for receive_from
    for ( auto& endpoint : endpoints )
    {
        if (endpoint.address().to_string() != "185.37.101.229")
            continue;

        utility::GenericBigEndianBuffer< 2048 > buffer;
        std::cout << "> endpoint used: " << endpoint << std::endl;

        boost::asio::io_service io_service;
        udp::socket socket( io_service );
        socket.open( udp::v4() );
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
        buffer.updateDataWritten( socket.receive_from( boost::asio::buffer( buffer.getDataForWriting(), 16 ), endpoint ) );
        int receivedTransactionId = 0;
        buffer >> action >> receivedTransactionId >> connectionId;
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
        //  84      32 - bit integer  IP address      0 // IP address set to 0. Response received to the sender of this packet
        //  88      32 - bit integer  key
        //  92      32 - bit integer  num_want - 1 // default
        //  96      16 - bit integer  port
    
        action = 1;
        buffer << connectionId << action << transactionId;

        const auto& info_hash = rootMetaInfo.getHashInfo();
        std::cout << "SHA1: " << utility::sha1_to_string( info_hash ) << std::endl;

        buffer.writeArray( info_hash );
        // urlencoded 20-byte string used as a unique ID for the client, generated by the client at startup. This is allowed to be any value, and may be binary data.
        // There are currently no guidelines for generating this peer ID. However, one may rightly presume that it must at least be unique for your local machine,
        // thus should probably incorporate things like process ID and perhaps a timestamp recorded at startup. See peer_id below for common client encodings of this field.
        buffer.writeString( "peer_id_pas_de_sens", 20 );

        long long downloaded = 0;
        long long left = 0;
        long long uploaded = 0;
        int evnt = 2; // start downloading
        int ipAddress = 0;
        auto key = utility::RandomGenerator< int >::instance().generate(); // randomized by client / unique by lan
        int num_want = -1;
        short port = socket.remote_endpoint().port();

        buffer << downloaded << left << uploaded << evnt << ipAddress << key << num_want << port;
        socket.send_to( boost::asio::buffer( buffer.getDataForReading(), buffer.size() ), endpoint );

        buffer.clear();

        // RECEIVE ANNOUNCE RESPONSE
        // Offset      Size            Name            Value
        //  0           32 - bit integer  action          1 // announce
        //  4           32 - bit integer  transaction_id
        //  8           32 - bit integer  interval
        //  12          32 - bit integer  leechers
        //  16          32 - bit integer  seeders
        //  20 + 6 * n  32 - bit integer  IP address
        //  24 + 6 * n  16 - bit integer  TCP port
        //  20 + 6 * N
        // (Can go up to 74 torents)
        buffer.updateDataWritten( socket.receive_from( boost::asio::buffer( buffer.getDataForWriting(), buffer.capacity() ), endpoint ) );
        if ( buffer.size() < 160 )
            throw std::invalid_argument( "Announce answer is less than 160 bytes" );
        std::cout << "!!!! " << buffer.size() << std::endl;

        int interval = 0; // in seconds
        int leechers = 0;
        int seeders = 0;
        buffer >> action >> receivedTransactionId >> interval >> leechers >> seeders;

        std::cout << "leechers: " << leechers << " | seeders: " << seeders << std::endl;
    }
}

BOOST_AUTO_TEST_SUITE_END() // ! TrackerTestSuite
