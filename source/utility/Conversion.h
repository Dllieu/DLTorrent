//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_CONVERSION_H__
#define __TORRENT_CONVERSION_H__

#include <boost/utility/string_ref.hpp>

namespace utility
{
    // cheap str_to_numeric as bencoding numeric are easy to parse (no exponential, only power of 10, no digit separator, etc...)
    unsigned int    naive_uint_conversion( const boost::string_ref& stringRef );
    long long       naive_ll_conversion( const boost::string_ref& stringRef );
}

#endif // ! __TORRENT_CONVERSION_H__
