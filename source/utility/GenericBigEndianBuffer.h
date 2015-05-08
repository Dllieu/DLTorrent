//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_GENERICBIGENDIANBUFFER_H__
#define __PARSING_GENERICBIGENDIANBUFFER_H__

#include <boost/endian/conversion.hpp>
#include <array>

namespace utility
{
    // Bufferize and store literals in big endian
    // - operator>> will retrieve a specific literal in native endian
    // - getData... will retrieve the underlying data in big endian
    template < size_t N >
    class GenericBigEndianBuffer
    {
    public:
        GenericBigEndianBuffer()
            : writeIndex_( 0 )
            , readIndex_( 0 )
        {}

        const char*    getDataForReading() const
        {
            return buffer_.data();
        }

        char*           getDataForWriting( size_t dataToBeWritten )
        {
            clearIndex();
            writeIndex_ = dataToBeWritten;
            return buffer_.data();
        }

        size_t      size() const
        {
            return writeIndex_;
        }

        void    clear()
        {
            buffer_.fill( 0 );
            clearIndex();
        }

        template < typename T >
        GenericBigEndianBuffer< N >&     operator<<( T t )
        {
            static_assert( std::is_integral< T >::value, "T is not an integral" );
            write( boost::endian::native_to_big( t ) );
            return *this;
        }

        template <>
        GenericBigEndianBuffer< N >&     operator<< < char >( char c )
        {
            return operator<<( static_cast< unsigned char >( c ) );
        }

        template <>
        GenericBigEndianBuffer< N >&     operator<< < bool >( bool b )
        {
            write( b );
            return *this;
        }

        template < typename T >
        GenericBigEndianBuffer< N >&  operator>>( T& t )
        {
            static_assert( std::is_integral< T >::value, "T is not an integral" );
            t = boost::endian::big_to_native( read< T >() );
            return *this;
        }

        template <>
        GenericBigEndianBuffer< N >&     operator>> < char >( char& c )
        {
            return operator>>( ( unsigned char& ) c );
        }

        template <>
        GenericBigEndianBuffer< N >&     operator>> < bool >( bool& b )
        {
            b = read< bool >();
            return *this;
        }

    private:
        void    clearIndex() const
        {
            writeIndex_ = 0;
            readIndex_ = 0;
        }

        template < typename T >
        void    write( T bigEndian )
        {
            if ( writeIndex_ + sizeof( T ) >= N )
                throw std::out_of_range( "Buffer full" );

            *reinterpret_cast< T* >( buffer_.data() + writeIndex_ ) = bigEndian;
            writeIndex_ += sizeof( T );
        }

        template < typename T >
        T    read()
        {
            T result;

            if ( readIndex_ + sizeof( T ) >= writeIndex_ + 1 )
                throw std::out_of_range( "Buffer empty" );

            result = *reinterpret_cast< T* >( buffer_.data() + readIndex_ );
            readIndex_ += sizeof( T );

            if ( readIndex_ == writeIndex_ )
                clear();

            return result;
        }

    private:
        std::array< char, N >  buffer_;
        mutable size_t         writeIndex_;
        mutable size_t         readIndex_;
    };
}

#endif // ! __PARSING_GENERICBIGENDIANBUFFER_H__
