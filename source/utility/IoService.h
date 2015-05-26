//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __UTILITY_IOSERVICE_H__
#define __UTILITY_IOSERVICE_H__

#include <boost/asio/io_service.hpp>

#include "utility/ThreadSafeSingleton.h"

namespace utility
{
    class IoService : public boost::asio::io_service
                    , public utility::ThreadSafeSingleton< IoService >
    {
    };
}

#endif // ! __UTILITY_IOSERVICE_H__
