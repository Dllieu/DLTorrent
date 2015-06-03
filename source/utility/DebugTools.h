//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __UTILITY_DEBUGTOOLS_H__
#define __UTILITY_DEBUGTOOLS_H__

#include <boost/asio/ip/udp.hpp>
#include <vector>
#include <sstream>

namespace utility
{
    inline std::string     generate_wireshark_filter( const std::vector< boost::asio::ip::udp::endpoint >& endpoints )
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

    template < typename T >
    class TypeDisplayer; // Do not define this template : will raise a compilation error stating the type T
}

#endif // ! __UTILITY_DEBUGTOOLS_H__
