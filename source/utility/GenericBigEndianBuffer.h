//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __UTILITY_GENERICBIGENDIANBUFFER_H__
#define __UTILITY_GENERICBIGENDIANBUFFER_H__

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

        size_t  capacity() const
        {
            return N;
        }

        bool        empty() const
        {
            return writeIndex_ == 0;
        }

        size_t      size() const
        {
            return writeIndex_;
        }

        void    clear()
        {
            if ( empty() )
                return;

            buffer_.fill( 0 );
            clearIndex();
        }

        const char*    getDataForReading() const
        {
            return buffer_.data();
        }

        // should always be called with updateDataWritten
        char*           getDataForWriting()
        {
            clearIndex();
            writeIndex_ = 0; // to be updated by updateDataWritten
            return buffer_.data();
        }

        // very ugly
        void            updateDataWritten( size_t dataWritten )
        {
            writeIndex_ = dataWritten;
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

        template < typename T, size_t N >
        void    writeArray( const std::array< T, N >& a )
        {
            for ( auto i = 0; i < a.size(); ++i )
                operator<<( a[ i ] );
        }

        void    writeString( const std::string& s, size_t sizeToWrite )
        {
            auto i = static_cast< size_t >( 0 );
            for ( ; i < sizeToWrite && i < s.size(); ++i )
                operator<<( s[ i ] );

            for ( ; i < sizeToWrite; ++i )
                operator<< < unsigned char >( 0 );
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

        void     readString( std::string& s, size_t sizeToRead )
        {
            s.resize( sizeToRead );

            auto lastNonTerminationChar = static_cast< size_t >( 0 );
            for ( auto i = 0; i < sizeToRead; ++i )
            {
                operator>>( s[ i ] );
                if ( s[ i ] != 0 )
                    lastNonTerminationChar = i;
            }

            s.resize( lastNonTerminationChar + 1 ); // + \0
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

#endif // ! __UTILITY_GENERICBIGENDIANBUFFER_H__
