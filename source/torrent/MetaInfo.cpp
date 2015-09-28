//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <locale>

#include "MetaInfo.h"

using namespace torrent;

namespace
{
    std::ostream&    apply_padding( std::ostream& os, size_t padding )
    {
        std::fill_n( std::ostream_iterator< char >( os ), padding, '-' );
        if ( padding > 0 )
            os << ' ';
        return os;
    }

    class OutputVisitor : public boost::static_visitor< void >
    {
    public:
        OutputVisitor( std::ostream& os, size_t padding )
            : os_( os )
            , padding_( padding )
        {
            // NOTHING
        }

        void    operator()( long long v ) const
        {
            apply_padding( os_, padding_ ) << v << std::endl;
        }

        void    operator()( const std::string& v ) const
        {
            if ( std::find_if( v.begin(), v.end(), [] ( unsigned char c ) { return ! isprint( c ); } ) != v.end() )
                apply_padding( os_, padding_ ) << "(binary...)" << std::endl;
            else
                apply_padding( os_, padding_ ) << v << std::endl;
        }

        void    operator()( const std::vector< MetaInfo >& v ) const
        {
            for ( const auto& e : v )
                boost::apply_visitor( OutputVisitor( os_, padding_ ), e );
        }

        void    operator()( const MetaInfoDictionary& v ) const
        {
            for ( const auto& e : v )
            {
                operator()( e.first );
                boost::apply_visitor( OutputVisitor( os_, padding_ + 1 ), e.second );
            }
        }

    private:
        std::ostream&   os_;
        size_t          padding_;
    };
}

std::ostream&   operator<<( std::ostream& os, const torrent::MetaInfo& metaInfo )
{
    boost::apply_visitor( OutputVisitor( os, 0 ), metaInfo );
    return os;
}
