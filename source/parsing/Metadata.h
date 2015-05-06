//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_METADATA_H__
#define __PARSING_METADATA_H__

#define BOOST_VARIANT_NO_FULL_RECURSIVE_VARIANT_SUPPORT
#include <boost/variant.hpp>
#undef BOOST_VARIANT_NO_FULL_RECURSIVE_VARIANT_SUPPORT

#include <unordered_map>
#include <string>

namespace parsing
{
    using Metadata = boost::make_recursive_variant< long long,
                                                    std::string,
                                                    std::vector< boost::recursive_variant_ >,
                                                    std::unordered_map< std::string, boost::recursive_variant_ > >::type;
}

#endif // ! __PARSING_METADATA_H__
