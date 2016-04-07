//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/use_future.hpp>
#pragma warning( pop )

#include <boost/dynamic_bitset.hpp>

#include <thread>
#include <iostream>
#include <unordered_set>

#include "utility/GenericBigEndianBuffer.h"
#include "utility/IoService.h"
#include "utility/TypeTraits.h"
#include "utility/Conversion.h"

#include "Peer.h"
#include "PieceInfo.h"

using namespace torrent;

namespace
{
    size_t TEST_INDEX = 0;

    enum class Timeout : int
    {
        Connect = 2,
        Handshake = 4,
        PostHandshake = 10,
    };

    enum class PeerMessage : char
    {
        Choke = 0,
        Unchoke = 1,
        Interested = 2, // send
        NotInterested = 3,
        Have = 4,
        BitField = 5, // send / receive
        Request = 6,
        Piece = 7, // send
    };

    // faire map de functor (qui prenne un BufferIn)

    template < size_t N >
    utility::GenericBigEndianBuffer< N >&     operator<<( utility::GenericBigEndianBuffer< N >& buffer, PeerMessage peerMessage )
    {
        buffer << utility::enum_cast( peerMessage );
        return buffer;
    }
}

namespace
{
    template < typename T >
    bool    ensure_valid_future( std::future< T >& future, int timeout )
    {
       return future.wait_for( std::chrono::seconds( timeout ) ) == std::future_status::ready;
    }
}

#define SHOW_DEBUG true

// One more thing to note about message passing – there seems to be no guarantee that messages will come in discrete packets containing only a single entire message.
// This means that you might end up with a long bytestring from a peer containing several messages, or you might end up with a bytestring
// from a peer that only has the length prefix for a message and the rest of the message will arrive in a later packet.
// You will need some way to deal with this inconsistency.The length prefix can be very helpful here to determine how much data you expect to have.
struct Peer::PImpl
{
    // TODO : update endpoints once in a while (when we have tried all endpoint -> notify Tracker to request more)
    PImpl( const std::vector< bai::tcp::endpoint >& endpoints, const std::array< char, 20 >& hashInfo, const PieceInfo& pieceInfo )
        : socket_( utility::IoService::instance() )
        , endpoints_( endpoints )
        , hashInfo_( hashInfo )
        , pieceInfo_( pieceInfo )
        , bitField_( pieceInfo.size() ) //!!!!!
        , isChocked_( true )
        , isInterested_( false )
        , timeout_( 5 )
    {
        // NOTHING
    }

    // todo : factorize
    bool    connect( const bai::tcp::endpoint& endpoint )
    {
        socket_.open( bai::tcp::v4() );
        if ( ! ::ensure_valid_future( socket_.async_connect( endpoint, boost::asio::use_future ), 2 ) )
            return false;

        if ( SHOW_DEBUG )
            std::cout << "******* Connected to " << endpoint << "\n";
        resetState();
        prepareHandshake(); // all prepare_ ... will send message or update internal infos

        return read()
            && parseHandshake();
    }

    bool    send()
    {
        if ( SHOW_DEBUG )
        std::cout << "=== SENDING " << bufferSend_.size() << " bytes " << std::endl;
        if ( ! ::ensure_valid_future( boost::asio::async_write( socket_, boost::asio::buffer( bufferSend_.getDataForReading(), bufferSend_.size() ), boost::asio::use_future ), timeout_ ) )
            return false;

        if ( SHOW_DEBUG )
        std::cout << "=== SENDING OK" << std::endl;

        bufferSend_.clear();
        return true;
    }

    bool    read()
    {
        if ( SHOW_DEBUG )
        std::cout << "-> READ" << std::endl;
        // unknown size
        //auto receiveFuture = boost::asio::async_read( socket_, boost::asio::buffer( bufferReceive_.getDataForWritingTESTTTTTTTTTTTTTTTTTTTTTTTT(), bufferReceive_.capacity() ), boost::asio::use_future );
        auto receiveFuture = socket_.async_read_some( boost::asio::buffer( bufferReceive_.getDataForWritingTESTTTTTTTTTTTTTTTTTTTTTTTT(), bufferReceive_.capacity() ), boost::asio::use_future );
        if ( ! ::ensure_valid_future( receiveFuture, timeout_ ) )
            return false;

        try
        {
            auto s = receiveFuture.get();
            bufferReceive_.updateDataWritten( s );
            if ( SHOW_DEBUG )
            std::cout << "%%%%%%%%%%%%%%%%%%%%%%%% READ " << s << " bytes" << std::endl;
        }
        catch ( const boost::system::system_error& error )
        {
            if ( SHOW_DEBUG )
            std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRASH (async_read_some): " << error.what() << std::endl;
            return false;
        }
        return true;
    }

    // try all endpoint on the vector -> should do a connect + handshake! then start peer processing
    void    try_endpoints()
    {
        if ( endpoints_.empty() || TEST_INDEX >= endpoints_.size() )
        {
            if ( SHOW_DEBUG )
            std::cout << "No endpoint to connect to..." << std::endl;
            return;
        }

        // do a queue et pop ?
        auto endpoint = endpoints_[ TEST_INDEX++ ];
        resetState();

        std::cout << "Trying " << endpoint << "...\n";
        
        if ( !connect( endpoint ) )
        {
            socket_.close();
            try_endpoints();
            return;
        }

        // TODO:
        // (OK) 1 - send proper bitfield (format got us to be rejected)
        // (OK) 2 - ensure we receive bitfield from host (with proper size)
        // (OK) 3 - send interest
        // (OK) 4 - receive unchocked
        // (OK) 5 - send request
        // (KO) 6 - receive piece (we close the connection before receiving the data chunk i.e. 16 * 1024, raise the timeout to 5 mins in unchoke handling message ??)
        // ... - request piece
        
        timeout_ = 60;
        prepareBitfield();

        for ( ;; )
        {
            while (parsePackets())
                ;

            if ( !read() )
                break;
        }

        socket_.close();
        try_endpoints();
    }

    // Slow method
    void    prepareBitfield()
    {
        auto v = utility::bitset_to_bytes( bitField_ );
        bufferSend_ << ( v.size() + 1 ) << utility::enum_cast( PeerMessage::BitField );
        bufferSend_.writeDynamicArray( v );

        if ( SHOW_DEBUG )
            std::cout << "PREPARE BITFIELD (size: " << v.size() << ")" << std::endl;

        send();
    }

    bool    parsePackets()
    {
        if ( bufferReceive_.size() < sizeof( int ) )
        {
            if ( bufferReceive_.empty() )
                bufferReceive_.clear();
            else
                std::cout << "Message less than a int" << std::endl;

            return false;
        }

        auto length = static_cast< int >( bufferReceive_ );
        if ( length > bufferReceive_.size() )
        {
            std::cout << "Message is truncated (waiting for next packet): " << bufferReceive_.size() << " / " << length << std::endl;
            // reset index
            bufferReceive_.rewindReadIndex( sizeof( int ) );
            return false;
        }

        if ( length < 0 )
        {
            std::cout << "invalid message length" << std::endl;
            socket_.close();
            return false;
        }

        if ( !length )
        {
            std::cout << "----------- RECEIVE KEEP ALIVE ---------------------" << std::endl;
            prepareKeepAlive(); // should only send one every 60 second
            return true;
        }

        // TODO
        // https://github.com/mpetazzoni/ttorrent/blob/master/core/src/main/java/com/turn/ttorrent/client/peer/PeerExchange.java
        auto messageType = static_cast< char >( bufferReceive_ );
        static int nbTimeReceive = 1;
        --length;
        //std::cout << "@@@@@@@@@@@@@@ Received message type: " << +messageType << " (size: " << length << " | number " << nbTimeReceive++ << ")" << std::endl;

        switch ( messageType )
        {
            case PeerMessage::Have:
                parseHave();
                break;

            case PeerMessage::Choke:
                parseChoke();
                break;

            case PeerMessage::Unchoke:
                parseUnchoke();
                break;

            case PeerMessage::BitField:
                return parseBitfield( length );

            default:
                bufferReceive_.skipBytes( length );
                std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Received unknown message type: " << +messageType << std::endl;
        }

        return true;
    }


#define EXIT_IF_INVALID_SOCKET( ERROR ) if ( exitAndConnectIfInvalidSocket( ERROR, false ) ) return
#define EXIT_AND_CONNECT_IF_INVALID_SOCKET( ERROR ) if ( exitAndConnectIfInvalidSocket( ERROR, true ) ) return

    // This enables a peer to block another peers request for data
    // <len=0001><id=0>
    void    parseChoke()
    {
        std::cout << "CHOKE received ######################################################################" << std::endl;
        isChocked_ = true;
        socket_.close();// ?????
    }

    // Unblock peer, and if they are still interested in the data, upload will begin.
    // <len=0001><id=1>
    void    parseUnchoke()
    {
        std::cout << "UNCHOKE received %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        isChocked_ = false;
        timeout_ = 5 * 60;

        if ( isInterested_ )
        {
            auto firstPiece = peerBitField_.find_first();
            if ( firstPiece != boost::dynamic_bitset<>::npos )
                prepare_request( (int) firstPiece, 0 ); // in theory must wait for server that he unchoke us, but never receive such message
        }
    }

    // Handle updates in piece availability from a peer's HAVE message. When this happens, we need to mark that piece as available from the peer.
    // Details the pieces that peer currently has.
    // <len=0005><id=4><piece index>
    void    parseHave()
    {
        auto index = static_cast< int >( bufferReceive_ );
        havePieceIndex_.insert( index );

        if ( SHOW_DEBUG )
        std::cout << "RECEIVED HAVE piece index: " << index << std::endl;
    }

    // Sent immediately after handshaking. Optional, and only sent if client has pieces.
    // Variable length, X is the length of bitfield. Payload represents pieces that have been successfully downloaded.
    // <len=0001+X><id=5><bitfield>
    bool    parseBitfield(size_t length)
    {
        auto expectedLength = static_cast< int >( bitField_.size() / 8 + ( bitField_.size() % 8 != 0 ? 1 : 0 ) );
        if ( length != expectedLength )
        {
            socket_.close();
            return false;
        }

        // todo: get bufferReceive_ in buffer -> process by size anyway -> put in Convertion then add test
        boost::dynamic_bitset<> peerBitset( bitField_.size() );
        for ( auto i = 0; i < peerBitset.size(); i += 8 )
        {
            auto c = static_cast< char >( bufferReceive_ );
            for ( auto j = 0; j < 8 && i + j < peerBitset.size(); ++j )
                peerBitset.set( i + j, ( c >> ( 7 - i ) ) & 1 );
        }

        peerBitField_ = std::move( peerBitset );

        std::string buffer;
        boost::to_string( peerBitField_, buffer );
        //if ( SHOW_DEBUG )
        std::cout << "RECEIVED BITFIELD: " << buffer << std::endl;

        // just send interested + request piece needed
        auto firstPiece = peerBitField_.find_first();
        if ( firstPiece != boost::dynamic_bitset<>::npos )
            prepareInterested();
        else
            prepareNotInterested();

        return true;
    }

    // Hanshake
    //<pstrlen><pstr><reserved><info_hash><peer_id>
    //
    // pstrlen:     string length of <pstr>, as a single raw byte
    // pstr:        string identifier of the protocol
    // reserved :   eight( 8 ) reserved bytes.A ll current implementations use all zeroes. Each bit in these bytes can be used to change the behavior of the protocol.
    // info_hash :  20 - byte SHA1 hash of the info key in the metainfo file.This is the same info_hash that is transmitted in tracker requests.
    // peer_id :    20 - byte string used as a unique ID for the client.This is usually the same peer_id that is transmitted in tracker requests( but not always e.g.an anonymity option in Azureus ).
    void    prepareHandshake()
    {
        static std::string protocolUsed( "BitTorrent protocol" ); // todo ?
        static size_t protocolUsedSize = protocolUsed.size();

        bufferSend_ << static_cast< uint8_t >( protocolUsedSize );
        bufferSend_.writeString( protocolUsed, protocolUsedSize );

        bufferSend_ << static_cast< uint64_t >( 0 ); // reserved bytes

        bufferSend_.writeArray( hashInfo_ );
        bufferSend_.writeString( /*peerId_*/"-DL0501-zzzzz", 20 ); // todo

        std::cout << "PREPARE HANDSHAKE" << std::endl;
        send();
    }

    // TODO : NE PAS LE METTRE ICI
    // Hanshake
    //<pstrlen><pstr><reserved><info_hash><peer_id>
    //
    // pstrlen:     string length of <pstr>, as a single raw byte
    // pstr:        string identifier of the protocol
    // reserved :   eight( 8 ) reserved bytes.A ll current implementations use all zeroes. Each bit in these bytes can be used to change the behavior of the protocol.
    // info_hash :  20 - byte SHA1 hash of the info key in the metainfo file.This is the same info_hash that is transmitted in tracker requests.
    // peer_id :    20 - byte string used as a unique ID for the client.This is usually the same peer_id that is transmitted in tracker requests( but not always e.g.an anonymity option in Azureus ).
    bool    parseHandshake()
    {
        if ( bufferReceive_.size() < 49 )
        {
            std::cout << "UNEXPECTED SIZE: " << bufferReceive_.size() << std::endl;
            std::cout << "HANDSHAKE KO" << std::endl;
            return false;
        }

        auto protocolUsedSize = static_cast< uint8_t >( bufferReceive_ );
        std::string protocolUsed = bufferReceive_.readString( protocolUsedSize );
        auto reservedBytes = static_cast< uint64_t >( bufferReceive_ );

        auto hashInfo = bufferReceive_.readArray< char, 20 >();
        std::string peerId = bufferReceive_.readString( 20 );

        auto isCorrectHash = hashInfo_ == hashInfo;

        std::cout << "HANDSHAKE OK" << std::endl;
        std::cout << "Hash is: " << (isCorrectHash ? "OK" : "KO") << std::endl;
        std::cout << "Peer protocol: " << protocolUsed << std::endl;
        std::cout << "Peer id: " << peerId << std::endl;

        if ( !isCorrectHash )
            return false;

        return true;
        // continue process hjere -> actually create a valid peer at that point -> give him the socket
        //setupAsyncRead();
    }

    void    resetState()
    {
        timeout_ = 5;
        isChocked_ = true;
        isInterested_ = false;
        havePieceIndex_.clear();

        bufferSend_.clear();
        bufferReceive_.clear();
    }

    bool    parseKeepAlive()
    {
        std::cout << "keep alive ok" << std::endl;

        bufferSend_.clear();
        prepareKeepAlive();
        return send();
    }

    void    parse_request()
    {
//        std::cout << "***** request result, with " << bytesTransferred << " bytes" << std::endl;
    }

    void    prepareKeepAlive()
    {
        std::cout << "SETUP KEEP ALIVE" << std::endl;
        bufferSend_ << 0;

        send();
    }

    // This enables a peer to block another peers request for data
    // <len=0001><id=0>
    void    prepareChoke()
    {
        bufferSend_ << 1 << utility::enum_cast( PeerMessage::Choke );
        std::cout << "SETUP CHOKE" << std::endl;

        send();
    }

    // Unblock peer, and if they are still interested in the data, upload will begin.
    // <len=0001><id=1>
    void    prepareUnchoke()
    {
        bufferSend_ << 1 << utility::enum_cast( PeerMessage::Unchoke );
        std::cout << "SETUP UNCHOKE" << std::endl;

        send();
    }

    // A user is interested if a peer has the data they require.
    // <len=0001><id=2>
    void    prepareInterested()
    {
        std::cout << "SETUP INTERESTED" << std::endl;

        isInterested_ = true;
        bufferSend_ << 1 << utility::enum_cast( PeerMessage::Interested );

        send();
    }

    // The peer does not have any data required.
    // <len = 0001><id = 3>
    void    prepareNotInterested()
    {
        std::cout << "SETUP NOT INTERESTED" << std::endl;
        bufferSend_ << 1 << utility::enum_cast( PeerMessage::NotInterested );

        // stop connec?
        socket_.close();
    }

    // Fixed length, used to request a block of pieces. The payload contains integer values specifying the index, begin location and length
    // <len=0013><id=6><index><begin><length>
    void    prepare_request(int index, int byteOffset)
    {
        // request: <len = 0013><id = 6><index><begin><length>
        // The request message is fixed length, and is used to request a block.The payload contains the following information :
        // index : integer specifying the zero - based piece index
        // begin : integer specifying the zero - based byte offset within the piece
        // length : integer specifying the requested length.

        bufferSend_ << 13 << utility::enum_cast( PeerMessage::Request ) << index << byteOffset << 16 * 1024/*length*/;//length should be 16 * 1024
        std::cout << "!!!!!!! PREPARE REQUEST: piece " << index << std::endl;

        send();
    }

    // Sent together with request messages. Fixed length, X is the length of the block. The payload contains integer values specifying the index, begin location and length.
    // <len=0009+X><id=7><index><begin><block>
    void    prepare_piece()
    {
        // The piece message is variable length, where X is the length of the block.The payload contains the following information :
        // index : integer specifying the zero - based piece index
        // begin : integer specifying the zero - based byte offset within the piece
        // block : block of data, which is a subset of the piece specified by index.
        //bufferSend_ << ( 9 + X ) << PeerMessage::Piece << /*piece index*/ << /*begin*/ << /*block*/;
    }

    // Fixed length, used to cancel block requests. payload is the same as ‘request’. Typically used during ‘end game’ mode.
    // <len = 13><id = 8><index><begin><length>
    void    prepare_cancel()
    {
        //bufferSend_ << 13 << PeerMessage::Cancel << /*piece index*/ << /*begin*/ << /*length*/;
    }


public:
    const std::array< char, 20 >                hashInfo_;

private:
    bai::tcp::socket                            socket_;
    std::vector< bai::tcp::endpoint >           endpoints_;
    const PieceInfo&                            pieceInfo_;

    bool                                        isChocked_;
    bool                                        isInterested_;
    int                                         timeout_;

    std::unordered_set<int>                     havePieceIndex_; // todo: remove
    boost::dynamic_bitset<>                     bitField_;
    boost::dynamic_bitset<>                     peerBitField_;


    // must reduce size corresponding of the max size message
    utility::GenericBigEndianBuffer< 32 * 1024 >     bufferReceive_; // need at least 16 * 1024 to handle one chunk
    utility::GenericBigEndianBuffer< 2048 >     bufferSend_;
};

#undef EXIT_IF_INVALID_SOCKET
#undef EXIT_AND_CONNECT_IF_INVALID_SOCKET


// check like http://codereview.stackexchange.com/questions/3770/bittorrent-peer-protocol-messages

// pour test, on prend une lsite ip/port, on les bourre tous
Peer::Peer( const std::vector< bai::tcp::endpoint >& endpoints, const std::array< char, 20 >& hashInfo, const PieceInfo& pieceInfo )
    : pimpl_( std::make_unique< PImpl >( endpoints, hashInfo, pieceInfo ) )
{
    // NOTHING
}

Peer::~Peer() = default;
Peer::Peer( Peer&& ) = default;
Peer& Peer::operator=( Peer&& ) = default;

void    Peer::connect()
{
    pimpl_->try_endpoints();
}
