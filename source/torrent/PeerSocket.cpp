#include "PeerSocket.h"

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/write.hpp>
#include <boost/asio/use_future.hpp>
#pragma warning( pop )

#include <future>

#include "utility/IoService.h"
#include "utility/Logger.h"

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
    LOG_INFO << "Trying " << endpoint_ << "...";

    socket_.open( bai::tcp::v4() ); // ???
    if ( !::ensure_valid_future( socket_.async_connect( endpoint_, boost::asio::use_future ), timeout_ ) )
        return false;

    LOG_INFO << "******* Connected to " << endpoint_;

    receiveBuffer_.clear();
    sendBuffer_.clear();
    return true;
}

bool    PeerSocket::send()
{
    LOG_DEBUG << "=== SENDING " << sendBuffer_.size() << " bytes ";
    if ( !::ensure_valid_future( boost::asio::async_write( socket_, boost::asio::buffer( sendBuffer_.getDataForReading(), sendBuffer_.size() ), boost::asio::use_future ), timeout_ ) )
        return false;

    LOG_DEBUG << "=== SENDING OK";

    sendBuffer_.clear();
    return true;
}

bool    PeerSocket::receive()
{
    LOG_DEBUG << "-> READ";

    auto receiveFuture = socket_.async_read_some( boost::asio::buffer( receiveBuffer_.getDataForWritingTESTTTTTTTTTTTTTTTTTTTTTTTT(), receiveBuffer_.capacity() ), boost::asio::use_future );
    if ( !::ensure_valid_future( receiveFuture, timeout_ ) )
        return false;

    try
    {
        auto s = receiveFuture.get();
        receiveBuffer_.updateDataWritten( s );
        LOG_DEBUG << "%%%%%%%%%%%%%%%%%%%%%%%% READ " << s << " bytes";
    }
    catch ( const boost::system::system_error& error )
    {
        LOG_DEBUG << "%%%%%%%%%%%%%%%%%%%%%%%%%%%% CRASH (async_read_some): " << error.what();
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
        if ( receiveBuffer_.size() >= sizeof( int ) )
        {
            auto length = static_cast< int >( receiveBuffer_ );
            if ( length < 0 )
            {
                LOG_ERROR << "invalid message length: " << length;
                return false;
            }

            auto underlyingMessageSize = receiveBuffer_.size();
            receiveBuffer_.rewindReadIndex( sizeof( int ) ); // reset index as we will we need the length to handle the messages
            if ( length <= underlyingMessageSize )
                return true;

            LOG_INFO << "Message is truncated (waiting for next packet): " << underlyingMessageSize << " / " << length;
        }

        if ( !receive() )
            return false;
    }
}
