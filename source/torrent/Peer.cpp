//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <thread>
#include <iostream>

#include "utility/GenericBigEndianBuffer.h"

#include "utility/IoService.h"
#include "Peer.h"

using namespace torrent;

// http://www.morehawes.co.uk/the-bittorrent-protocol

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
        KeepAlive = -1, // custom index : keep alive doesnt have an id in the protocol
        Choke,
        Unchoke,
        Interested,
        NotInterested,
        Have,
        BitField,
        Request,
        Piece,
    };

    template < size_t N >
    utility::GenericBigEndianBuffer< N >&     operator<<( utility::GenericBigEndianBuffer< N >& buffer, PeerMessage peerMessage )
    {
        buffer << static_cast< int >( peerMessage );
        return buffer;
    }
}


// One more thing to note about message passing – there seems to be no guarantee that messages will come in discrete packets containing only a single entire message.
// This means that you might end up with a long bytestring from a peer containing several messages, or you might end up with a bytestring
// from a peer that only has the length prefix for a message and the rest of the message will arrive in a later packet.
// You will need some way to deal with this inconsistency.The length prefix can be very helpful here to determine how much data you expect to have.


// connection Logic : OK (single peer)
// message logic : ko (only one way here to the peer)

// Design from http://www.boost.org/doc/libs/1_45_0/doc/html/boost_asio/example/timeouts/async_tcp_client.cpp
// TODO : decouper :
//        1 - find vliad endpoint process : loop et fais un connect
//                                        - si connect succeed -> try handshake, si handshake succeed : OK
//        2 - start peer process
struct Peer::PImpl
{
    PImpl( const std::vector< bai::tcp::endpoint >& endpoints, const std::array< char, 20 >& hashInfo )
        : socket_( utility::IoService::instance() )
        , deadline_( utility::IoService::instance() )
        , endpoints_( endpoints )
        , hashInfo_( hashInfo )
    {}

    // try all endpoint on the vector -> should do a connect + handshake! then start peer processing
    void    try_endpoint()
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


#define EXIT_IF_INVALID_SOCKET( ERROR ) if ( exitAndConnectIfInvalidSocket( ERROR, false ) ) return
#define EXIT_AND_CONNECT_IF_INVALID_SOCKET( ERROR ) if ( exitAndConnectIfInvalidSocket( ERROR, true ) ) return

    // TODO : NE PAS LE METTRE ICI
    void    parse_handshake( const boost::system::error_code& errorCode, std::size_t bytesTransferred )
    {
        EXIT_AND_CONNECT_IF_INVALID_SOCKET( errorCode );
        if ( bytesTransferred < 68 )
        {
            std::cout << "UNEXPECTED SIZE: " << bytesTransferred << std::endl;
            deadline_.expires_from_now( boost::posix_time::seconds( 2 ) );
            socket_.async_read_some( boost::asio::buffer( bufferReceive_.getDataForWriting(), bufferReceive_.capacity() ), [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { parse_handshake( errorCode, bytesTransferred ); } );
            return;
        }

        bufferReceive_.updateDataWritten( bytesTransferred );
        std::cout << "read succeed" << std::endl;

        std::string protocolUsed;
        auto n = static_cast< char >( bufferReceive_ );
        bufferReceive_.readString( protocolUsed, n );
        std::cout << "Peer protocol: " << protocolUsed << std::endl;

        auto useless = static_cast< uint64_t >( bufferReceive_ );

        std::array< char, 20 > hash;
        bufferReceive_.readArray( hash );


        std::string peerId;
        bufferReceive_.readString( peerId, 20 );
        std::cout << "Peer id: " << peerId << std::endl;

        // continue process hjere -> actually create a valid peer at that point -> give him the socket
        start_leeching();
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

    void    onConnect( const boost::system::error_code& errorCode, const bai::tcp::endpoint& endpoint )
    {
        EXIT_AND_CONNECT_IF_INVALID_SOCKET( errorCode );

        std::cout << "Connected to " << endpoint << "\n";

        // Try handshake
        bufferSend_.clear();
        bufferReceive_.clear();

        prepare_handshake();

        deadline_.expires_from_now( boost::posix_time::seconds( 2 ) );
        socket_.async_read_some( boost::asio::buffer( bufferReceive_.getDataForWriting(), bufferReceive_.capacity() ), [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { parse_handshake( errorCode, bytesTransferred ); } );
        /*boost::asio::async_read( socket_, boost::asio::buffer( bufferReceive_.getDataForWriting(), 68 ),
                                 [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { parse_handshake( errorCode, bytesTransferred ); } );*/
        setupAsyncWrite();
    }

    // TODO: put previous code somewhere else as it only concern the connection logic



    void    start_leeching()
    {
        setupMessage( PeerMessage::KeepAlive );
    }


    bool    exitAndConnectIfInvalidSocket( const boost::system::error_code& errorCode, bool mustConnect )
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

        if ( mustConnect )
            try_endpoint();

        return true;
    }

    void    setupAsyncWrite()
    {
        boost::asio::async_write( socket_, boost::asio::buffer( bufferSend_.getDataForReading(), bufferSend_.size() ),
                                  [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred )
        {
            EXIT_IF_INVALID_SOCKET( errorCode );
            std::cout << "async write succeed" << std::endl;
        } );
    }

    void    onAsyncReadResult( const boost::system::error_code& errorCode, std::size_t bytesTransferred )
    {
        EXIT_AND_CONNECT_IF_INVALID_SOCKET( errorCode );
        bufferReceive_.updateDataWritten( bytesTransferred );

        auto length = static_cast< int >( bufferReceive_ );
        if ( ! length )
            parse_keep_alive();
        else
        {
            // TODO
            // https://github.com/mpetazzoni/ttorrent/blob/master/core/src/main/java/com/turn/ttorrent/client/peer/PeerExchange.java
            auto messageType = static_cast< int >( bufferReceive_ );
            std::cout << "Received message type: " << messageType << std::endl;
        }

        if ( bufferReceive_.size() > 0 )
            onAsyncReadResult( errorCode, bufferReceive_.size() );
        else
            setupAsyncRead();
    }

    void    setupAsyncRead()
    {
        deadline_.expires_from_now( boost::posix_time::seconds( 10 ) );
        socket_.async_read_some( boost::asio::buffer( bufferReceive_.getDataForWriting(), bufferReceive_.capacity() ),
                                 [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { onAsyncReadResult( errorCode, bytesTransferred ); } );
    }

    void    parse_keep_alive()
    {
        std::cout << "keep alive ok" << std::endl;
        //setupMessage( PeerMessage::Request );
    }

    void    parse_request()
    {
//        std::cout << "***** request result, with " << bytesTransferred << " bytes" << std::endl;
    }

    void    prepare_keep_alive()
    {
        bufferSend_ << 0;
    }

    void    prepare_choke()
    {
        bufferSend_ << 1 << PeerMessage::Choke;
    }

    void    prepare_unchoke()
    {
        bufferSend_ << 1 << PeerMessage::Unchoke;
    }

    void    prepare_interested()
    {
        bufferSend_ << 1 << PeerMessage::Interested;
    }

    void    prepare_not_interested()
    {
        bufferSend_ << 1 << PeerMessage::NotInterested;
    }

    void    prepare_have()
    {
        //bufferSend_ << 5 << PeerMessage::Have << /*piece index*/;
    }

    void    prepare_bitfield()
    {
        // The bitfield message may only be sent immediately after the handshaking sequence is completed, and before any other messages are sent. It is optional, and need not be sent if a client has no pieces.
        // The bitfield message is variable length, where X is the length of the bitfield.The payload is a bitfield representing the pieces that have been successfully downloaded.The high bit in the first byte corresponds to piece index 0. Bits that are cleared indicated a missing piece, and set bits indicate a valid and available piece.Spare bits at the end are set to zero.
        // Some clients( Deluge for example ) send bitfield with missing pieces even if it has all data.Then it sends rest of pieces as have messages.They are saying this helps against ISP filtering of BitTorrent protocol.It is called lazy bitfield.
        // A bitfield of the wrong length is considered an error.Clients should drop the connection if they receive bitfields that are not of the correct size, or if the bitfield has any of the spare bits set.
        //bufferSend_ << ( 1 + ?? ) << PeerMessage::Bitfield << /*bitfield*/;
    }

    void    prepare_request()
    {
        // request: <len = 0013><id = 6><index><begin><length>
        // The request message is fixed length, and is used to request a block.The payload contains the following information :
        // index : integer specifying the zero - based piece index
        // begin : integer specifying the zero - based byte offset within the piece
        // length : integer specifying the requested length.

        bufferSend_ << 13 << PeerMessage::Request << 1 << 0 << 16 * 1024/*length*/;//length should be 16 * 1024
    }

    void    prepare_piece()
    {
        // The piece message is variable length, where X is the length of the block.The payload contains the following information :
        // index : integer specifying the zero - based piece index
        // begin : integer specifying the zero - based byte offset within the piece
        // block : block of data, which is a subset of the piece specified by index.
        //bufferSend_ << ( 9 + X ) << PeerMessage::Piece << /*piece index*/ << /*begin*/ << /*block*/;
    }

    void    prepare_cancel()
    {
        //bufferSend_ << 13 << PeerMessage::Cancel << /*piece index*/ << /*begin*/ << /*length*/;
    }

    #undef PREPARE_ASYNC_READ

    void    setupMessage( PeerMessage peerMessage )
    {
        bufferSend_.clear();
        bufferReceive_.clear();

        switch ( peerMessage )
        {
            case PeerMessage::KeepAlive:
                //std::this_thread::sleep_for( std::chrono::seconds( 10 ) ); // test purpose (gros probleme si on bourrine, ca coupe a partir d'un moment, toutes les autres actions vont fail)
                prepare_keep_alive();
                break;

            case PeerMessage::Request:
                prepare_request();
                break;

            default:
                std::cout << "Unhandled PeerMessage: " << static_cast< int >( peerMessage ) << std::endl;
                return;
        }

        setupAsyncRead();
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

#undef EXIT_IF_INVALID_SOCKET
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
    pimpl_->try_endpoint();
}
