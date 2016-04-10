//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#include "Peer.h"

#include <boost/dynamic_bitset.hpp>
#include <unordered_set>

#include "utility/TypeTraits.h"
#include "utility/Conversion.h"
#include "utility/Logger.h"

#include "PieceInfo.h"
#include "PeerSocket.h"

using namespace torrent;
namespace bai = boost::asio::ip;

namespace
{
    enum class PeerMessage : char
    {
        Choke = 0, // receive
        Unchoke = 1, // receive
        Interested = 2, // send
        NotInterested = 3,
        Have = 4, // receive
        BitField = 5, // send / receive
        Request = 6, // send
        Piece = 7, // receive
    };
}

struct Peer::PImpl
{
    PImpl( bai::tcp::endpoint& endpoint, const std::array< char, 20 >& hashInfo, const PieceInfo& pieceInfo )
        : isChocked_( true )
        , isInterested_( false )
        , hashInfo_( hashInfo )
        , pieceInfo_( pieceInfo )
        , socket_( endpoint, 2 )
        , bitField_( pieceInfo.size() )
    {
        // NOTHING
    }

    void    start()
    {
        isChocked_ = true;
        isInterested_ = false;
        havePieceIndex_.clear();

        if ( !socket_.connect() )
            return;

        socket_.setTimeout( 6 );
        if ( ! verifyHandshake() )
            return;

        socket_.setTimeout( 30 );
        if ( ! sendBitfield() )
            return;

        for ( ;; )
            if ( !socket_.waitReceiveUntruncatedMessage()
                || ! handleReceivedMessage() )
                return;
    }

    bool    verifyHandshake()
    {
        if ( ! sendHandshake() )
            return false;

        size_t protocolUsedSize;
        while ( socket_.receive() )
            if ( socket_.receiveBuffer().size() > 1 )
            {
                protocolUsedSize = static_cast< char >( socket_.receiveBuffer() );
                do
                {
                    if ( socket_.receiveBuffer().size() >= 48 /*+ protocolUsedSize*/ )
                        return handleHandshake( protocolUsedSize );

                } while ( socket_.receive() );
                return false;
            }

        return false;
    }

    // Hanshake
    //<pstrlen><pstr><reserved><info_hash><peer_id>
    //
    // pstrlen:     string length of <pstr>, as a single raw byte
    // pstr:        string identifier of the protocol
    // reserved :   eight( 8 ) reserved bytes.A ll current implementations use all zeroes. Each bit in these bytes can be used to change the behavior of the protocol.
    // info_hash :  20 - byte SHA1 hash of the info key in the metainfo file.This is the same info_hash that is transmitted in tracker requests.
    // peer_id :    20 - byte string used as a unique ID for the client.This is usually the same peer_id that is transmitted in tracker requests( but not always e.g.an anonymity option in Azureus ).
    bool    sendHandshake()
    {
        static std::string protocolUsed( "BitTorrent protocol" ); // todo ?
        static size_t protocolUsedSize = protocolUsed.size();

        auto& sendBuffer = socket_.sendBuffer();

        sendBuffer << static_cast< uint8_t >( protocolUsedSize );
        sendBuffer.writeString( protocolUsed, protocolUsedSize );

        sendBuffer << static_cast< uint64_t >( 0 ); // reserved bytes

        sendBuffer.writeArray( hashInfo_ );
        sendBuffer.writeString( /*peerId_*/"-DL0501-zzzzz", 20 ); // todo

        LOG_INFO << "PREPARE HANDSHAKE";

        return socket_.send();
    }

    // Hanshake
    //<pstrlen><pstr><reserved><info_hash><peer_id>
    //
    // pstrlen:     string length of <pstr>, as a single raw byte
    // pstr:        string identifier of the protocol
    // reserved :   eight( 8 ) reserved bytes.A ll current implementations use all zeroes. Each bit in these bytes can be used to change the behavior of the protocol.
    // info_hash :  20 - byte SHA1 hash of the info key in the metainfo file.This is the same info_hash that is transmitted in tracker requests.
    // peer_id :    20 - byte string used as a unique ID for the client.This is usually the same peer_id that is transmitted in tracker requests( but not always e.g.an anonymity option in Azureus ).
    // It is (49 + len(pstr)) bytes long.
    bool    handleHandshake( size_t protocolUsedSize )
    {
        auto& receiveBuffer = socket_.receiveBuffer();

        std::string protocolUsed = receiveBuffer.readString( protocolUsedSize );
        auto reservedBytes = static_cast< uint64_t >( receiveBuffer );

        auto hashInfo = receiveBuffer.readArray< char, 20 >();
        std::string peerId = receiveBuffer.readString( 20 );

        auto isCorrectHash = hashInfo_ == hashInfo;

        LOG_INFO << "Hash is: " << ( isCorrectHash ? "OK" : "KO" );
        LOG_INFO << "Peer protocol: " << protocolUsed;
        LOG_INFO << "Peer id: " << peerId;

        return isCorrectHash;
    }

    bool    handleReceivedMessage()
    {
        auto length = static_cast< int >( socket_.receiveBuffer() );
        if ( !length )
            return handleKeepAlive(); // should only send one every 60 second

        auto messageType = static_cast< char >( socket_.receiveBuffer() );
        --length;
        
        switch ( messageType )
        {
            case PeerMessage::Have:
                return handleHave();

            case PeerMessage::Choke:
                return handleChoke();

            case PeerMessage::Unchoke:
                return handleUnchoke();

            case PeerMessage::BitField:
                return handleBitfield( length );

            default:
                socket_.receiveBuffer().skipBytes( length );
                LOG_WARNING << "$ Received unknown message type: " << +messageType;
                return true;
        }
    }

    // Slow method
    bool    sendBitfield()
    {
        auto v = utility::bitset_to_bytes( bitField_ );
        LOG_DEBUG << "PREPARE BITFIELD (size: " << v.size() << ")";

        socket_.sendBuffer() << ( v.size() + 1 ) << utility::enum_cast( PeerMessage::BitField );
        socket_.sendBuffer().writeDynamicArray( v );

        return socket_.send();
    }

    // This enables a peer to block another peers request for data
    // <len=0001><id=0>
    bool    handleChoke()
    {
        LOG_INFO << "CHOKE received ######################################################################";
        isChocked_ = true;
        return false;
    }

    // Unblock peer, and if they are still interested in the data, upload will begin.
    // <len=0001><id=1>
    bool    handleUnchoke()
    {
        LOG_INFO << "UNCHOKE received %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
        if ( !isInterested_ )
            return false;


        isChocked_ = false;
        socket_.setTimeout( 5 * 60 );

        auto firstPiece = peerBitField_.find_first();
        if ( firstPiece == boost::dynamic_bitset<>::npos )
            return false;

        return sendRequest( (int) firstPiece, 0 ); // in theory must wait for server that he unchoke us, but never receive such message
    }

    // Handle updates in piece availability from a peer's HAVE message. When this happens, we need to mark that piece as available from the peer.
    // Details the pieces that peer currently has.
    // <len=0005><id=4><piece index>
    bool    handleHave()
    {
        auto index = static_cast< int >( socket_.receiveBuffer() );
        havePieceIndex_.insert( index );

        LOG_INFO << "RECEIVED HAVE piece index: " << index;
        return true;
    }

    // Sent immediately after handshaking. Optional, and only sent if client has pieces.
    // Variable length, X is the length of bitfield. Payload represents pieces that have been successfully downloaded.
    // <len=0001+X><id=5><bitfield>
    bool    handleBitfield( size_t length )
    {
        auto expectedLength = static_cast< int >( bitField_.size() / 8 + ( bitField_.size() % 8 != 0 ? 1 : 0 ) );
        if ( length != expectedLength )
            return false;

        // todo: get bufferReceive_ in buffer -> process by size anyway -> put in Convertion then add test
        boost::dynamic_bitset<> peerBitset( bitField_.size() );
        for ( auto i = 0; i < peerBitset.size(); i += 8 )
        {
            auto c = static_cast< char >( socket_.receiveBuffer() );
            for ( auto j = 0; j < 8 && i + j < peerBitset.size(); ++j )
                peerBitset.set( i + j, ( c >> ( 7 - i ) ) & 1 );
        }

        peerBitField_ = std::move( peerBitset );

        std::string buffer;
        boost::to_string( peerBitField_, buffer );
        LOG_INFO << "RECEIVED BITFIELD: " << buffer;

        // just send interested + request piece needed
        auto firstPiece = peerBitField_.find_first();
        if ( firstPiece == boost::dynamic_bitset<>::npos )
            return false; // not interested

        return sendInterested();
    }

    bool    handleKeepAlive()
    {
        LOG_DEBUG << "keep alive ok";
        return sendKeepAlive();
    }

    bool    sendKeepAlive()
    {
        LOG_INFO << "SETUP KEEP ALIVE";
        socket_.sendBuffer() << 0;
        return socket_.send();
    }

    // A user is interested if a peer has the data they require.
    // <len=0001><id=2>
    bool    sendInterested()
    {
        LOG_DEBUG << "SETUP INTERESTED";

        isInterested_ = true;
        socket_.sendBuffer() << 1 << utility::enum_cast( PeerMessage::Interested );

        return socket_.send();
    }

    // Fixed length, used to request a block of pieces. The payload contains integer values specifying the index, begin location and length
    // <len=0013><id=6><index><begin><length>
    bool    sendRequest( int index, int byteOffset )
    {
        // request: <len = 0013><id = 6><index><begin><length>
        // The request message is fixed length, and is used to request a block.The payload contains the following information :
        // index : integer specifying the zero - based piece index
        // begin : integer specifying the zero - based byte offset within the piece
        // length : integer specifying the requested length.

        socket_.sendBuffer() << 13 << utility::enum_cast( PeerMessage::Request ) << index << byteOffset << 16 * 1024/*length*/;//length should be 16 * 1024
        LOG_INFO << "!!!!!!! PREPARE REQUEST: piece " << index;

        return socket_.send();
    }

private:
    std::unordered_set<int>                     havePieceIndex_;
    bool                                        isChocked_;
    bool                                        isInterested_;

    const std::array< char, 20 >                hashInfo_;
    const PieceInfo&                            pieceInfo_;

    PeerSocket                                  socket_;

    boost::dynamic_bitset<>                     bitField_;
    boost::dynamic_bitset<>                     peerBitField_;
};

Peer::Peer( bai::tcp::endpoint& endpoint, const std::array< char, 20 >& hashInfo, const PieceInfo& pieceInfo )
    : pimpl_( std::make_unique< PImpl >( endpoint, hashInfo, pieceInfo ) )
{
    // NOTHING
}

Peer::~Peer() = default;
Peer::Peer( Peer&& ) = default;
Peer& Peer::operator=( Peer&& ) = default;

void    Peer::start()
{
    pimpl_->start();
}
