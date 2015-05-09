//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __UTILITY_DEBUGTOOLS_H__
#define __UTILITY_DEBUGTOOLS_H__

#include <boost/asio/ip/udp.hpp>
#include <vector>

namespace utility
{
    std::string     generate_wireshark_filter( const std::vector< boost::asio::ip::udp::endpoint >& endpoints );

    template < typename T >
}

#endif // ! __UTILITY_DEBUGTOOLS_H__

