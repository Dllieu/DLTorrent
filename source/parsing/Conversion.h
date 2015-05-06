//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_CONVERSION_H__
#define __PARSING_CONVERSION_H__

#include <boost/utility/string_ref.hpp>

namespace parsing
{
    unsigned int    naive_uint_conversion( const boost::string_ref& stringRef );
    long long       naive_ll_conversion( const boost::string_ref& stringRef );
}

#endif // ! __PARSING_CONVERSION_H__
