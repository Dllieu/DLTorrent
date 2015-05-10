//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>

#include "utility/Sha1Encoder.h"
#include "BEncoder.h"
#include "RootMetaInfo.h"

using namespace parsing;

namespace
{
    // TODO: use regexp and handle more formatting ? (do that once html is handled)
    void   push_udp_endpoint( std::vector< bai::udp::endpoint >& endpoints, const std::string& url, boost::asio::io_service& ioService )
    {
        static std::string expectedToStartWith( "udp://" );

        if ( !boost::starts_with( url, expectedToStartWith ) )
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

        bai::udp::resolver::query query( bai::udp::v4(), hostname, port );
        bai::udp::resolver resolver( ioService );

        try
        {
            bai::udp::resolver::iterator end;
            for ( auto it = resolver.resolve( query ); it != end; ++it )
                endpoints.emplace_back( bai::udp::endpoint( *it ) );
        }
        catch ( ... )
        {
            // Log -> hostname / port : unreachable
        }
    }

    std::vector< bai::udp::endpoint >   parse_endpoints_from_root_metainfo( const MetaInfoList& announceList )
    {
        boost::asio::io_service ioService;

        std::vector< bai::udp::endpoint > result;
        for ( const auto& subAnnounceList : announceList )
        {
            const auto& urlList = boost::get< MetaInfoList >( subAnnounceList );
            for ( const auto& url : urlList )
                push_udp_endpoint( result, boost::get< MetaInfoString >( url ), ioService );
        }
        return result;
    }

    uint64_t      parse_total_length( const MetaInfoDictionary& dataChunk )
    {
        auto result = static_cast< uint64_t >( 0 );
        const auto& files = boost::get< MetaInfoList >( dataChunk.at( "files" ) );
        for ( const auto& fileInformationMetaInfo : files )
        {
            const auto& fileInformation = boost::get< MetaInfoDictionary >( fileInformationMetaInfo );
            result += boost::get< MetaInfoInteger >( fileInformation.at( "length" ) );
        }
        return result;
    }
}

RootMetaInfo::RootMetaInfo( MetaInfoDictionary&& root )
    : root_( root )
    , announcers_( parse_endpoints_from_root_metainfo( boost::get< MetaInfoList >( root_.at( "announce-list" ) ) ) )
    , hashInfo_( utility::Sha1Encoder::instance().encode( BEncoder::encode( root_.at( "info" ) ) ) )
    , bytesToDownload_( parse_total_length( boost::get< MetaInfoDictionary >( root_.at( "info" ) ) ) )
{
    // NOTHING
}

void    RootMetaInfo::display() const
{
    std::cout << root_ << std::endl;
}
