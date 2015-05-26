//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_CUSTOMEXCEPTION_H__
#define __TORRENT_CUSTOMEXCEPTION_H__

#include <exception>

namespace utility
{

#define CUSTOM_EXCEPTION( EXCEPTION_NAME )\
    class EXCEPTION_NAME : public std::exception\
    {\
    public:\
        EXCEPTION_NAME( std::string error ) : error_( std::move( #EXCEPTION_NAME + ( ": " + error ) ) ) {}\
        const char*     what() const override \
        { \
            return error_.c_str(); \
        } \
    \
    private:\
        std::string error_;\
    }

    CUSTOM_EXCEPTION( invalid_metainfo_integer );
    CUSTOM_EXCEPTION( invalid_metainfo_string );
    CUSTOM_EXCEPTION( invalid_metainfo_list );
    CUSTOM_EXCEPTION( invalid_metainfo_dictionary );

#undef CUSTOM_EXCEPTION
}

#endif // ! __TORRENT_CUSTOMEXCEPTION_H__
