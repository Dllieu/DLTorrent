//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <thread>
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


// TODO : update endpoints once in a while (when we have tried all endpoint -> notify Tracker to request more)

namespace
{
    size_t TEST_INDEX = 0;

    enum class PeerMessage
    {
        Handshake,
        KeepAlive,

        // Custom messages
        Inactive
    };
}

// Design from http://www.boost.org/doc/libs/1_45_0/doc/html/boost_asio/example/timeouts/async_tcp_client.cpp
struct Peer::PImpl
{
    PImpl( const std::vector< bai::tcp::endpoint >& endpoints, const std::array< char, 20 >& hashInfo )
        : socket_( IoService::instance() )
        , deadline_( IoService::instance() )
        , endpoints_( endpoints )
        , hashInfo_( hashInfo )
    {}

    void    connect()
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
        socket_.async_connect( endpoint, [ this, endpoint ] ( const boost::system::error_code& errorCode ) { onConnectResult( errorCode, endpoint ); } );
    }

    bool    exitAndConnectIfInvalidSocket( const boost::system::error_code& errorCode )
    {
        if ( socket_.is_open() && ! errorCode )
            return false;

        if ( errorCode )
        {
            std::cout << "error: " << errorCode.message() << std::endl;
            socket_.close();
        }
        else
            std::cout << "action timed out\n" << std::endl;

        connect();
        return true;
    }

    #define EXIT_AND_CONNECT_IF_INVALID_SOCKET( ERROR ) if ( exitAndConnectIfInvalidSocket( ERROR ) ) return

    void    onConnectResult( const boost::system::error_code& errorCode, const bai::tcp::endpoint& endpoint )
    {
        EXIT_AND_CONNECT_IF_INVALID_SOCKET( errorCode );

        std::cout << "Connected to " << endpoint << "\n";
        setupMessage( PeerMessage::Handshake );
    }

    template < typename T >
    void    setupAsyncRead( T&& asyncHandler, std::size_t messageToReceiveSize )
    {
        deadline_.expires_from_now( boost::posix_time::seconds( 30 ) );
        boost::asio::async_read( socket_, boost::asio::buffer( bufferReceive_.getDataForWriting(), messageToReceiveSize ), std::forward< T >( asyncHandler ) );
    }

    void    onAsyncWriteResult( const boost::system::error_code& errorCode, std::size_t bytesTransferred )
    {
        EXIT_AND_CONNECT_IF_INVALID_SOCKET( errorCode );
        std::cout << "async write succeed" << std::endl;
    }

    void    setupAsyncWrite()
    {
        boost::asio::async_write( socket_, boost::asio::buffer( bufferSend_.getDataForReading(), bufferSend_.size() ),
                                  [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { onAsyncWriteResult( errorCode, bytesTransferred ); } );
    }

    void    onKeepAliveResult( const boost::system::error_code& errorCode, std::size_t bytesTransferred )
    {
        EXIT_AND_CONNECT_IF_INVALID_SOCKET( errorCode );

        std::cout << "keep alive ok" << std::endl;
        setupMessage( PeerMessage::KeepAlive );
    }

    void    onHandshakeResult( const boost::system::error_code& errorCode, std::size_t bytesTransferred )
    {
        EXIT_AND_CONNECT_IF_INVALID_SOCKET( errorCode );

        bufferReceive_.updateDataWritten( bytesTransferred );

        std::cout << "read succeed" << std::endl;

        char n = 0;
        bufferReceive_ >> n;
        std::string protocolUsed;
        bufferReceive_.readString( protocolUsed, n );
        std::cout << "Peer protocol: " << protocolUsed << std::endl;

        uint64_t useless = 0;
        bufferReceive_ >> useless;


        std::array< char, 20 > hash;
        bufferReceive_.readArray( hash );


        std::string peerId;
        bufferReceive_.readString( peerId, 20 );
        std::cout << "Peer id: " << peerId << std::endl;

        setupMessage( PeerMessage::KeepAlive );
    }

    void    prepare_keep_alive()
    {
        bufferSend_ << 0;
    }

    void    prepare_handshake()
    {
        // todo
        std::string protocolUsed( "BitTorrent protocol" );

        bufferSend_ << static_cast< char >( protocolUsed.size() );
        bufferSend_.writeString( protocolUsed, protocolUsed.size() );

        bufferSend_ << static_cast< uint64_t >( 0 ); // reserved bytes

        bufferSend_.writeArray( hashInfo_ );
        bufferSend_.writeString( /*peerId_*/"-DL0101-zzzzz", 20 ); // todo
    }

    void    setupMessage( PeerMessage peerMessage )
    {
        bufferSend_.clear();
        bufferReceive_.clear();

        switch ( peerMessage )
        {
            case PeerMessage::Handshake:
                setupAsyncRead( [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { onHandshakeResult( errorCode, bytesTransferred ); }, 68 );
                prepare_handshake();
                break;

            case PeerMessage::KeepAlive:
                //std::this_thread::sleep_for( std::chrono::seconds( 10 ) ); // test purpose (gros probleme si on bourrine, ca coupe a partir d'un moment, toutes les autres actions vont fail)
                setupAsyncRead( [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { onKeepAliveResult( errorCode, bytesTransferred ); }, 1 );
                prepare_keep_alive();
                break;

            default:
                std::cout << "Unhandled PeerMessage: " << static_cast< int >( peerMessage ) << std::endl;
                return;
        }

        setupAsyncWrite();
    }

    void    checkDeadline( const boost::system::error_code& errorCode )
    {
        // Check whether the deadline has passed. We compare the deadline against
        // the current time since a new asynchronous operation may have moved the
        // deadline before this actor had a chance to run.
        if ( deadline_.expires_at() > boost::asio::deadline_timer::traits_type::now() )
        {
            // Put the actor back to sleep.
            deadline_.async_wait( [ this ] ( const boost::system::error_code& errorCode ) { checkDeadline( errorCode ); } );
            return;
        }

        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled.
        socket_.close();

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        deadline_.expires_at( boost::posix_time::pos_infin );
    }


public:
    const std::array< char, 20 >                hashInfo_;

private:
    bai::tcp::socket                            socket_;
    boost::asio::deadline_timer                 deadline_;
    std::vector< bai::tcp::endpoint >           endpoints_;

    // must reduce size corresponding of the max size message
    utility::GenericBigEndianBuffer< 2048 >     bufferReceive_;
    utility::GenericBigEndianBuffer< 2048 >     bufferSend_;
};

#undef EXIT_AND_CONNECT_IF_INVALID_SOCKET


// check like http://codereview.stackexchange.com/questions/3770/bittorrent-peer-protocol-messages

// pour test, on prend une lsite ip/port, on les bourre tous
Peer::Peer( const std::vector< bai::tcp::endpoint >& endpoints, const std::array< char, 20 >& hashInfo )
    : pimpl_( std::make_unique< PImpl >( endpoints, hashInfo ) )
{
    // NOTHING
}

Peer::~Peer() = default;
// Default move constructors not supporter by VS2013
//Peer::Peer( Peer&& ) = default;
//Peer& Peer::operator=( Peer&& ) = default;

void    Peer::connect()
{
    pimpl_->connect();
}
