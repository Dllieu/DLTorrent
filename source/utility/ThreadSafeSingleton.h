//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __UTILITY_THREADSAFESINGLETON_H__
#define __UTILITY_THREADSAFESINGLETON_H__

#include <memory>
#include <mutex>

namespace utility
{
    template < typename T >
    class ThreadSafeSingleton
    {
    public:
        static T& instance()
        {
            static std::unique_ptr< T >     singleton;
            static std::once_flag           onceFlag;

            std::call_once( onceFlag, [] () { singleton.reset( new T ); } );
            return *singleton;
        }
    };

}

#endif /* ! __GENERIC_DESIGNPATTERN_THREADSAFESINGLETON_H__ */