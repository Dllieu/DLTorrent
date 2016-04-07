//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_METAINFO_H__
#define __TORRENT_METAINFO_H__

#define BOOST_VARIANT_NO_FULL_RECURSIVE_VARIANT_SUPPORT
    #include <boost/variant.hpp>
#undef BOOST_VARIANT_NO_FULL_RECURSIVE_VARIANT_SUPPORT

#include <unordered_map>
#include <string>

namespace torrent
{
    constexpr const size_t SHA1_LENGTH = 20;

    // handle binteger as a long long (at least 8 bytes) to handle large files (torrent for more than 4Gbytes)
    using MetaInfoInteger       = uint64_t;
    using MetaInfoString        = std::string;

    using MetaInfo              = boost::make_recursive_variant< MetaInfoInteger,
                                                                 MetaInfoString,
                                                                 std::vector< boost::recursive_variant_ >,
                                                                 std::unordered_map< std::string, boost::recursive_variant_ > >::type;

    using MetaInfoList          = std::vector< MetaInfo >;
    using MetaInfoDictionary    = std::unordered_map< std::string, MetaInfo >;
}

// Debug utils
std::ostream&   operator<<( std::ostream& os, const torrent::MetaInfo& metaInfo );

#endif // ! __TORRENT_METAINFO_H__
