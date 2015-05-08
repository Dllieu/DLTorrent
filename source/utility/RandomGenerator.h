//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_TRANSACTIONIDGENERATOR_H__
#define __PARSING_TRANSACTIONIDGENERATOR_H__

#include <random>

#include "ThreadSafeSingleton.h"

namespace utility
{
    template < typename T >
    class RandomGenerator : public ThreadSafeSingleton< RandomGenerator< T > >
    {
    public:
        RandomGenerator()
            : randomEngine_( std::random_device{}( ) )
            , uniformIntDistribution_( std::numeric_limits< T >::min(), std::numeric_limits< T >::max() )
        {}

        T  generate() const
        {
            return uniformIntDistribution_( randomEngine_ );
        }

    private:
        mutable std::default_random_engine      randomEngine_;
        std::uniform_int_distribution< T >    uniformIntDistribution_;

        static_assert( std::is_integral< T >::value, "Incompatible type for RandomGenerator" );
    };
}

#endif // ! __PARSING_TRANSACTIONIDGENERATOR_H__
