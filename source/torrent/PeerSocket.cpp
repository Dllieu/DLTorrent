#include "PeerSocket.h"

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/write.hpp>
#include <boost/asio/use_future.hpp>
#pragma warning( pop )

#include <iostream>
#include <future>

#include "utility/IoService.h"

using namespace torrent;
namespace bai = boost::asio::ip;

PeerSocket::PeerSocket( boost::asio::ip::tcp::endpoint& endpoint, int timeout )
    : socket_( utility::IoService::instance() )
    , endpoint_( endpoint )
    , timeout_( timeout )
{
    // NOTHING
}

namespace
{
    template < typename T >
    bool    ensure_valid_future( std::future< T >& future, int timeout )
    {
        return future.wait_for( std::chrono::seconds( timeout ) ) == std::future_status::ready;
    }
}

bool    PeerSocket::connect()
{
    std::cout << "Trying " << endpoint_ << "...\n";

    socket_.open( bai::tcp::v4() ); // ???
    if ( !::ensure_valid_future( socket_.async_connect( endpoint_, boost::asio::use_future ), timeout_ ) )
        return false;

    std::cout << "******* Connected to " << endpoint_ << "\n";

    receiveBuffer_.clear();
    sendBuffer_.clear();
    return true;
}

bool    PeerSocket::send()
{
    std::cout << "=== SENDING " << sendBuffer_.size() << " bytes " << std::endl;
    if ( !::ensure_valid_future( boost::asio::async_write( socket_, boost::asio::buffer( sendBuffer_.getDataForReading(), sendBuffer_.size() ), boost::asio::use_future ), timeout_ ) )
        return false;

    std::cout << "=== SENDING OK" << std::endl;

    sendBuffer_.clear();
    return true;
}

bool    PeerSocket::receive()
{
    std::cout << "-> READ" << std::endl;

    auto receiveFuture = socket_.async_read_some( boost::asio::buffer( receiveBuffer_.getDataForWritingTESTTTTTTTTTTTTTTTTTTTTTTTT(), receiveBuffer_.capacity() ), boost::asio::use_future );
    if ( !::ensure_valid_future( receiveFuture, timeout_ ) )
        return false;

    try
    {
        auto s = receiveFuture.get();
        receiveBuffer_.updateDataWritten( s );
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%% READ " << s << " bytes" << std::endl;
    }
    catch ( const boost::system::system_error& error )
    {
        std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRASH (async_read_some): " << error.what() << std::endl;
        return false;
    }
    return true;
}

// There seems to be no guarantee that messages will come in discrete packets containing only a single entire message.
// This means that you might end up with a long bytestring from a peer containing several messages, or you might end up with a bytestring
// from a peer that only has the length prefix for a message and the rest of the message will arrive in a later packet.
// You will need some way to deal with this inconsistency.The length prefix can be very helpful here to determine how much data you expect to have.
bool    PeerSocket::waitReceiveUntruncatedMessage()
{
    for ( ;; )
    {
        if ( !receive() )
            return false;

        if ( receiveBuffer_.size() < sizeof( int ) )
        {
            std::cout << "Message less than a int" << std::endl;
            continue;
        }

        auto length = static_cast< int >( receiveBuffer_ );
        if ( length < 0 )
        {
            std::cout << "invalid message length: " << length << std::endl;
            return false;
        }

        if ( length > receiveBuffer_.size() )
        {
            std::cout << "Message is truncated (waiting for next packet): " << receiveBuffer_.size() << " / " << length << std::endl;
            receiveBuffer_.rewindReadIndex( sizeof( int ) );
            continue;
        }

        receiveBuffer_.rewindReadIndex( sizeof( int ) ); // reset index as we will we need the length to handle the messages
        return true;
    }
}
