//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <sstream>

#include "BEncoder.h"

using namespace parsing;

namespace
{
    class EncoderVisitor : public boost::static_visitor< void >
    {
    public:
        EncoderVisitor( std::stringstream& ss )
            : ss_( ss )
        {
            // NOTHING
        }

        void    operator()( long long v ) const
        {
            ss_ << "i" << v << "e";
        }

        void    operator()( const std::string& v ) const
        {
            ss_ << v.size() << ":" << v;
        }

        void    operator()( const std::vector< Metadata >& v ) const
        {
            ss_ << "l";
            for ( const auto& e : v )
                boost::apply_visitor( EncoderVisitor( ss_ ), e );
            ss_ << "e";
        }

        void    operator()( const std::unordered_map< std::string, Metadata >& v ) const
        {
            ss_ << "d";
            for ( const auto& e : v )
            {
                operator()( e.first );
                boost::apply_visitor( EncoderVisitor( ss_ ), e.second );
            }
            ss_ << "e";
        }

    private:
        mutable std::stringstream&  ss_;
    };
}

/*static*/ std::string  BEncoder::encode( const Metadata& metadata )
{
    std::stringstream ss;
    boost::apply_visitor( EncoderVisitor( ss ), metadata );
    return ss.str();
}
