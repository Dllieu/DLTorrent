#include "TrackerSocket.h"

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/write.hpp>
#include <boost/asio/use_future.hpp>
#pragma warning( pop )

#include <iostream>

#include "utility/IoService.h"

using namespace torrent;
namespace bai = boost::asio::ip;

TrackerSocket::TrackerSocket( bai::udp::endpoint& endpoint )
    : socket_( utility::IoService::instance() )
    , endpoint_( endpoint )
{
    // NOTHING
}

namespace
{
    template < typename T >
    bool    ensure_valid_future( std::future< T >& future, const char* errorMessage )
    {
        if ( future.wait_for( std::chrono::seconds( 2 ) ) == std::future_status::ready )
            return true;

        std::cout << "TracketSocket timeout: " << errorMessage << std::endl;
        return false;
    }
}

bool    TrackerSocket::connect()
{
    socket_.open( bai::udp::v4() ); // ????
    if ( !::ensure_valid_future( socket_.async_connect( endpoint_, boost::asio::use_future ), "Couldn't connect to announcer" ) )
        return false;

    buffer_.clear();
    return true;
}

bool    TrackerSocket::send()
{
    if ( !::ensure_valid_future( socket_.async_send_to( boost::asio::buffer( buffer_.getDataForReading(), buffer_.size() ), endpoint_, boost::asio::use_future ), "send failed" ) )
        return false;

    buffer_.clear();
    return true;
}

bool    TrackerSocket::receive()
{
    auto receiveFuture = socket_.async_receive_from( boost::asio::buffer( buffer_.getDataForWriting(), buffer_.capacity() ), endpoint_, boost::asio::use_future );
    if ( !::ensure_valid_future( receiveFuture, "receive failed" ) )
        return false;

    try
    {
        buffer_.updateDataWritten( receiveFuture.get() );
    }
    catch ( const boost::system::system_error& error )
    {
        std::cout << "Couldn't receive from announcer: " << error.what() << std::endl;
        return false;
    }
    return true;
}
