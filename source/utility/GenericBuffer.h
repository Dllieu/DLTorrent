//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_GENERICBUFFER_H__
#define __PARSING_GENERICBUFFER_H__

#include <array>

namespace utility
{
    template < size_t N >
    class GenericBuffer
    {
    public:
        GenericBuffer()
            : writeIndex_( 0 )
            , readIndex_( 0 )
        {}

        template < typename T >
        GenericBuffer< N >&     operator<<( T t )
        {
            static_assert( std::is_integral< T >::value, "T is not an integral" );

            if ( writeIndex_ + sizeof( T ) >= N )
                throw std::out_of_range( "Buffer full" );

            *reinterpret_cast< T* >( buffer_.data() + writeIndex_ ) = t;
            writeIndex_ += sizeof( T );
            return *this;
        }

        template< typename T >
        GenericBuffer< N >&     write_pod( const T& t )
        {
            static_assert( std::is_pod< T >::value, "T is not a pod" );

            if ( writeIndex_ + sizeof( T ) >= N )
                throw std::out_of_range( "Buffer full" );

            *reinterpret_cast< T* >( buffer_.data() + writeIndex_ ) = t;
            writeIndex_ += sizeof( T );
            return *this;
        }


        template < typename T >
        GenericBuffer< N >&  operator>>( T& t )
        {
            static_assert( std::is_integral< T >::value || std::is_pod< T >::value, "T is not an integral, nor a pod" );

            if ( readIndex_ + sizeof( T ) >= writeIndex_ + 1 )
                throw std::out_of_range( "Buffer empty" );

            t = *reinterpret_cast< T* >( buffer_.data() + readIndex_ );
            readIndex_ += sizeof( T );

            if ( readIndex_ == writeIndex_ )
                clear();

            return *this;
        }

        void    clear()
        {
            writeIndex_ = 0;
            readIndex_ = 0;
        }

    private:
        std::array< char, N >       buffer_;
        mutable size_t              writeIndex_;
        mutable size_t              readIndex_;
    };
}

#endif // ! __PARSING_GENERICBUFFER_H__
