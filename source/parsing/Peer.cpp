//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#include <iostream>

#include "utility/GenericBigEndianBuffer.h"

#include "IoService.h"
#include "Peer.h"

using namespace parsing;

// http://stackoverflow.com/questions/26683500/continuously-streaming-data-with-boostasio-without-closing-socket

// who do the connect :
// try_connect_peer : https://github.com/libtorrent/libtorrent/blob/d11376bbe3bae1d620bfba7f2d0d319bca556c90/src/session_impl.cpp
// implemented in torrent.cpp -> policy.connect_one_peer

// actual peer
// https://github.com/libtorrent/libtorrent/blob/d11376bbe3bae1d620bfba7f2d0d319bca556c90/include/libtorrent/policy.hpp
//https://github.com/libtorrent/libtorrent/blob/d11376bbe3bae1d620bfba7f2d0d319bca556c90/src/bt_peer_connection.cpp
// https://github.com/libtorrent/libtorrent/blob/master/src/peer_connection.cpp

// interessant :
// - 1817 : peer_connection::incoming_piece(
// 

namespace
{
    size_t TEST_INDEX = 0;
}


// check like http://codereview.stackexchange.com/questions/3770/bittorrent-peer-protocol-messages

// pour test, on prend une lsite ip/port, on les bourre tous
Peer::Peer( const std::vector< bai::tcp::endpoint >& endpoints )
    : socket_( IoService::instance() )
    , deadline_( IoService::instance() )
    , endpoints_( endpoints ) // need to udpate it once in a while (i.e. when attempting every endpoints)
{
    // NOTHING
}


// Design from http://www.boost.org/doc/libs/1_45_0/doc/html/boost_asio/example/timeouts/async_tcp_client.cpp
void    Peer::checkDeadline( const boost::system::error_code& errorCode )
{
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if ( deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now() )
    {
        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled.
        socket_.close();

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        deadline_.expires_at( boost::posix_time::pos_infin );
    }

    // comprend pas l'interet de le resttart ici, cense etre fait au niveau du Peer::onConnect sinon on va possiblement faitre pop 2 thread sur le meme truc
    // Put the actor back to sleep.
    //deadline_.async_wait( [ this ] ( const boost::system::error_code& errorCode ) { checkDeadline( errorCode ); } );
}

void    Peer::onConnect( const boost::system::error_code& errorCode, const bai::tcp::endpoint& endpoint )
{
    // in case timeout from deadline_
    if ( ! socket_.is_open() )
    {
        std::cout << "Connect timed out\n";

        // Try the next available endpoint.
        connect();
        return;
    }

    // Check if the connect operation failed before the deadline expired.
    if ( errorCode )
    {
        std::cout << "Connect error: " << errorCode.message() << "\n";

        // We need to close the socket used in the previous connection attempt
        // before starting a new one.
        socket_.close();

        // Try the next available endpoint.
        connect();
        return;
    }

    // Otherwise we have successfully established a connection.
    std::cout << "Connected to " << endpoint << "\n";
    socket_.close();
    connect();
    //// Start the input actor.
    //start_read();

    //// Start the heartbeat actor.
    //start_write();
}

void    Peer::connect()
{
    if ( endpoints_.empty() || TEST_INDEX >= endpoints_.size() )
    {
        std::cout << "No endpoint to connect to..." << std::endl;
        return;
    }

    deadline_.async_wait( [ this ] ( const boost::system::error_code& errorCode ) { checkDeadline( errorCode ); } );

    // do a queue et pop ?
    auto endpoint = endpoints_[ TEST_INDEX++ ];
    std::cout << "Trying " << endpoint << "...\n";
    deadline_.expires_from_now( boost::posix_time::seconds( 2 ) );
    socket_.async_connect( endpoint, [ this, endpoint ] ( const boost::system::error_code& errorCode ) { onConnect( errorCode, endpoint ); } );
}
