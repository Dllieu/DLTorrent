#pragma once

#include <type_traits>

namespace utility
{
    template < typename ENUM_TYPE >
    constexpr auto enum_cast( ENUM_TYPE v )
    {
        return static_cast<std::underlying_type_t< ENUM_TYPE >>( v );
    }
}