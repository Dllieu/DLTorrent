//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <sstream>

#include "DebugTools.h"

std::string     utility::generate_wireshark_filter( const std::vector< boost::asio::ip::udp::endpoint >& endpoints )
{
    std::stringstream wiresharkSs;

    bool first = true;
    for ( const auto& endpoint : endpoints )
    {
        if ( first )
            first = false;
        else
            wiresharkSs << " || ";

        wiresharkSs << "ip.dst == " << endpoint.address() << " || ip.src == " << endpoint.address();
    }
    return wiresharkSs.str();
}