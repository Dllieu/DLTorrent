//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/algorithm/string/predicate.hpp>

#include "RootMetaInfo.h"
#include "BEncoder.h"
#include "utility/Sha1Encoder.h"

using namespace parsing;
namespace bai = boost::asio::ip;

namespace
{
    // TODO: use regexp and handle more formatting ? (do that once html is handled)
    void   push_udp_endpoint( std::vector< bai::udp::endpoint >& endpoints, const std::string& url )
    {
        static std::string expectedToStartWith( "udp://" );

        if ( ! boost::starts_with( url, expectedToStartWith ) )
            return;

        auto from = expectedToStartWith.size();
        auto to = url.find( ':', from );
        if ( to == std::string::npos )
            return;

        auto hostname = url.substr( from, to - from );
        from = to + 1;
        to = url.find( '/', from );
        if ( to == std::string::npos )
            return;

        auto port = url.substr( from, to - from );

        // TODO : PUT SOMEWHERE ELSE WHICH MAKE MORE SENSE
        boost::asio::io_service io_service;
        bai::udp::resolver::query query( bai::udp::v4(), hostname, port );
        bai::udp::resolver resolver( io_service );

        try
        {
            bai::udp::resolver::iterator end;
            for ( auto it = resolver.resolve( query ); it != end; ++it )
                endpoints.emplace_back( bai::udp::endpoint( *it ) );
        }
        catch (...)
        {
            // Log -> hostname / port : unreachable
        }
    }

    // todo handle HTTP
    std::vector< bai::udp::endpoint >   parse_endpoint_from_root_metainfo( const MetaInfoDictionary& root )
    {
        std::vector< bai::udp::endpoint > result;

        const auto& announceList = boost::get< MetaInfoList >( root.at( "announce-list" ) );
        for ( const auto& subAnnounceList : announceList )
        {
            const auto& urlList = boost::get< MetaInfoList >( subAnnounceList );
            for ( const auto& url : urlList )
                push_udp_endpoint( result, boost::get< std::string >( url ) );
        }

        return result;
    }
}

RootMetaInfo::RootMetaInfo( MetaInfoDictionary&& root )
    : root_( root )
    , announcers_( parse_endpoint_from_root_metainfo( root_ ) )
    , hashInfo_( utility::Sha1Encoder::instance().encode( BEncoder::encode( root_[ "info" ] ) ) )
{
    // NOTHING
}

const std::vector< bai::udp::endpoint >&    RootMetaInfo::getAnnouncers() const
{
    return announcers_;
}

const std::array< char, 20 >&               RootMetaInfo::getHashInfo() const
{
    return hashInfo_;
}
