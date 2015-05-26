//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <atomic>
#include <iostream>
#include <thread>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#include "utility/GenericBigEndianBuffer.h"
#include "utility/Sha1Encoder.h"
#include "utility/RandomGenerator.h"
#include "utility/IoService.h"

#include "RootMetaInfo.h"
#include "Tracker.h"
#include "Peer.h"

using namespace torrent;

#define UNINITIALIZED_CONNECTION_ID 0x41727101980

namespace
{
    enum class TrackerMessage
    {
        // API actions
        Connect = 0,
        Announce = 1,
        Scrap = 2,
        Error = 3,

        // Custom messages
        Inactive
    };
}

// http://en.wikipedia.org/wiki/Tracker_scrap
// http://www.bittorrent.org/beps/bep_0015.html
// TODO
// 1 - on fait un announce pour obtenir la liste des peers (tres bandwith consuming)
// 2 - si endpoint accepte scraping, envoi de requete (sur plusieurs torrent) pour determiner si il y a + de seeder de dispo, si oui et qu'on telecharge pas au max, on peut refaire un announce sur le torrent en question
//   - sinon on attend n seconds (timeout donner par reponse announce) et on refait un announce

struct Tracker::PImpl
{
    PImpl( RootMetaInfo&& root )
        : root( root )
        , running( false )
        , socket_( utility::IoService::instance() )
    {}

    void    start()
    {
        std::vector< bai::udp::endpoint > endpoints = root.getAnnouncers();
        for ( auto& endpoint : endpoints )
        {
            // TEST PHASE : just use an endpoint who respond
            if ( endpoint.address().to_string() != "185.37.101.229" ) // hehe
            {
                currentEndpoint_ = endpoint;
                break;
            }
        }

        socket_.open( bai::udp::v4() );
        socket_.connect( currentEndpoint_ );

        prepare_connect();
        socket_.async_send_to( boost::asio::buffer( buffer_.getDataForReading(), buffer_.size() ), currentEndpoint_,
                               [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { onSend( errorCode, bytesTransferred ); } );
    }

    void    onReceive( const boost::system::error_code& error, std::size_t bytesTransferred )
    {
        // TODO: check error
        buffer_.updateDataWritten( bytesTransferred );
        // check result, if weird result stop

        if ( ! read_result_and_prepare() )
        {
            running.store( false, std::memory_order_relaxed );
            return;
        }

        socket_.async_send_to( boost::asio::buffer( buffer_.getDataForReading(), buffer_.size() ), currentEndpoint_,
                               [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { onSend( errorCode, bytesTransferred ); } );
    }

    void    onSend( const boost::system::error_code& /*error*/, std::size_t /*bytes_transferred*/ )
    {
        // TODO: check error
        buffer_.clear();
        socket_.async_receive_from( boost::asio::buffer( buffer_.getDataForWriting(), buffer_.capacity() ), currentEndpoint_,
                                    [ this ] ( const boost::system::error_code& errorCode, std::size_t bytesTransferred ) { onReceive( errorCode, bytesTransferred ); } );
    }

    // Always start by
    // Offset  Size            Name            Value
    //     0       32 - bit integer  action          0 // connect
    //     4       32 - bit integer  transaction_id
    //     8
    bool    read_result_and_prepare()
    {
        auto receivedAction = 0;
        auto receivedTransactionId = 0;

        buffer_ >> receivedAction >> receivedTransactionId;
        if ( receivedAction != action_
            || receivedTransactionId != transactionId_
            || ! parse_result() )
            return false;

        buffer_.clear();
        prepare_to_send();
        return true;
    }

    bool    parse_result()
    {
        switch ( action_ )
        {
        case TrackerMessage::Connect:
            return parse_connect();

        case TrackerMessage::Announce:
            return parse_announce();

        case TrackerMessage::Scrap:
            return parse_scrap();

        case TrackerMessage::Error:
            return parse_error();
        }
        return false;
    }

    void    prepare_to_send()
    {
        switch ( action_ )
        {
        case TrackerMessage::Connect:
            prepare_connect();
            break;

        case TrackerMessage::Announce:
            prepare_announce();
            break;

        case TrackerMessage::Scrap:
            prepare_scrap();
            break;
        }
    }

    // Offset  Size            Name            Value
    //     8       64 - bit integer  connection_id
    //     16
    bool    parse_connect()
    {
        if ( buffer_.size() < 8 )
            return false;

        std::cout << "received connect" << std::endl;
        buffer_ >> connectionId_;
        action_ = static_cast< decltype( action_ ) >( TrackerMessage::Announce ); // next step : announce

        return true;
    }

    // Handshake
    // The handshake is a required message and must be the first message transmitted by the client.It is( 49 + len( pstr ) ) bytes long.
    // handshake: <pstrlen><pstr><reserved><info_hash><peer_id>
    // pstrlen : string length of <pstr>, as a single raw byte
    // pstr : string identifier of the protocol
    // reserved : eight( 8 ) reserved bytes.All current implementations use all zeroes.Each bit in these bytes can be used to change the behavior of the protocol.An email from Bram suggests that trailing bits should be used first, so that leading bits may be used to change the meaning of trailing bits.
    // info_hash : 20 - byte SHA1 hash of the info key in the metainfo file.This is the same info_hash that is transmitted in tracker requests.
    // peer_id : 20 - byte string used as a unique ID for the client.This is usually the same peer_id that is transmitted in tracker requests( but not always e.g.an anonymity option in Azureus ).


    // The peer should immediately respond with his own handshake message, which takes the same form as yours.
    // If you received a peer_id in your tracker response, you should check that the peer_id provided by the peer in the handshake matches what you expect.
    // If it does not, you should close the connection.
    void test_stuff( std::vector< bai::udp::endpoint >& endpoints )
    {
        buffer_.clear();
        //bytes(chr(19))+"BitTorrent protocol00000000"+self.getInfoHash(torrentCont)+self.peer_id
        std::stringstream wiresharkSs;
        bool first = true;
        std::vector< bai::tcp::endpoint > tcpEndpoints;
        for ( auto& endpoint : endpoints )
        {
            if (first)
                first = false;
            else
                wiresharkSs << " || ";
            wiresharkSs << "ip.dst == " << endpoint.address() << " || ip.src == " << endpoint.address();
            tcpEndpoints.emplace_back( bai::tcp::endpoint( endpoint.address(), endpoint.port() ) );
        }

        // TODO : cleaner
        std::string debugggg = wiresharkSs.str();
        static Peer peer( tcpEndpoints, root.getHashInfo() );
        static bool stufffff = false; // just test
        if ( !stufffff )
        {
            peer.connect();
            stufffff = true;
        }
    }

    // Offset      Size            Name            Value
    //  8           32 - bit integer  interval
    //  12          32 - bit integer  leechers
    //  16          32 - bit integer  seeders
    //  20 + 6 * n  32 - bit integer  IP address
    //  24 + 6 * n  16 - bit integer  TCP port // + 16 bit for the last one as it might be aligned
    //  20 + 6 * N
    bool    parse_announce()
    {
        if ( buffer_.size() < 152 )
            throw std::invalid_argument( "Announce answer incorrect size" );

        std::cout << "!!!! " << buffer_.size() << std::endl;

        auto interval = static_cast< int >( buffer_ ); // in seconds
        auto leechers = static_cast< int >( buffer_ );
        auto seeders = static_cast< int >( buffer_ );

        std::cout << "Peers:" << std::endl;
        std::vector< bai::udp::endpoint > peerEndpoints;

        int ipAddress = 0;
        short port = 0;
        while ( buffer_.size() > 48 /*sizeof(ip) + sizeof(port)*/ )
        {
            buffer_ >> ipAddress >> port;

            peerEndpoints.emplace_back( bai::udp::endpoint( boost::asio::ip::address_v4( ipAddress ), port ) );
        }

        // to remove (test)
        test_stuff( peerEndpoints );

        std::cout << "Peers available: " << peerEndpoints.size() << std::endl;
        std::cout << "leechers: " << leechers << " | seeders: " << seeders << std::endl;

        // Test: Stop once receiving one announce
        action_ = static_cast< decltype( action_ ) >( TrackerMessage::Inactive );

        return true;
    }

    // can scrap up to 74 torrent at once
    bool    parse_scrap()
    {
        return false;
    }

    // Offset  Size            Name            Value
    //     8       string  message
    bool    parse_error()
    {
        if ( buffer_.size() < 8 )
            return false;

        std::string errorMessage;
        buffer_.readString( errorMessage, 8 );

        std::cout << "Received error from Tracker: " << errorMessage << std::endl;
        return false;
    }

    // Offset  Size            Name            Value
    //     0       64 - bit integer  connection_id   0x41727101980
    //     8       32 - bit integer  action          0 // connect
    //     12      32 - bit integer  transaction_id
    //     16
    void    prepare_connect()
    {
        buffer_ << static_cast< uint64_t >( UNINITIALIZED_CONNECTION_ID )
                << ( action_ = static_cast< decltype( action_ ) >( TrackerMessage::Connect ) )
                << ( transactionId_ = utility::RandomGenerator< int >::instance().generate() );
    }

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
    void    prepare_announce()
    {
        buffer_ << connectionId_
                << ( action_ = static_cast< decltype( action_ ) >( TrackerMessage::Announce ) )
                << transactionId_;

        std::cout << "Send Announce request with SHA1: " << utility::sha1_to_string( root.getHashInfo() ) << std::endl;

        buffer_.writeArray( root.getHashInfo() );
        // urlencoded 20-byte string used as a unique ID for the client, generated by the client at startup. This is allowed to be any value, and may be binary data.
        // There are currently no guidelines for generating this peer ID. However, one may rightly presume that it must at least be unique for your local machine,
        // thus should probably incorporate things like process ID and perhaps a timestamp recorded at startup. See peer_id below for common client encodings of this field.
        buffer_.writeString( /*peerId_*/"-DL0101-zzzzz", 20 );

        uint64_t downloaded = 0;
        uint64_t left = root.getBytesToDownload();
        uint64_t uploaded = 0;
        int evnt = 2; // start downloading
        int ipAddress = 0;
        auto key = utility::RandomGenerator< int >::instance().generate(); // randomized by client / unique by lan
        int num_want = -1;
        short port = socket_.remote_endpoint().port();

        buffer_ << downloaded << left << uploaded << evnt << ipAddress << key << num_want << port;
    }

    // can scrap up to 74 torrent at once
    void    prepare_scrap()
    {
    }

public:
    const RootMetaInfo                          root;
    std::atomic< bool >                         running;

private:
    utility::GenericBigEndianBuffer< 2048 >     buffer_; // must reduce size corresponding of the max size message

    bai::udp::socket                            socket_;

    bai::udp::endpoint                          currentEndpoint_;


    //
    int                                         transactionId_;
    uint64_t                                    connectionId_;
    int                                         action_;
};

Tracker::Tracker( RootMetaInfo&& root )
    : pimpl_( std::make_unique< PImpl >( std::move( root ) ) )
{}

Tracker::~Tracker() = default;
// Default move constructors not supporter by VS2013
//Tracker::Tracker( Tracker&& ) = default;
//Tracker& Tracker::operator=( Tracker&& ) = default;

const RootMetaInfo&     Tracker::getRootMetaInfo() const
{
    return pimpl_->root;
}

void    Tracker::start()
{
    pimpl_->start();
}
