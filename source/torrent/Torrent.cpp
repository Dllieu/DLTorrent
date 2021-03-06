//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/algorithm/string/predicate.hpp>
#include <cstdio>

#include "utility/Sha1Encoder.h"
#include "BEncoder.h"
#include "Torrent.h"

using namespace torrent;
namespace bai = boost::asio::ip;

namespace
{
    std::vector< bai::udp::endpoint >   parse_endpoints_from_root_metainfo( const MetaInfoList& announceList )
    {
        boost::asio::io_service ioService;

        char hostname[1024];
        auto port = static_cast< unsigned short >( 0 );
        std::vector< bai::udp::endpoint > result;
        for ( const auto& subAnnounceList : announceList )
            for ( const auto& metaUrl : boost::get< MetaInfoList >( subAnnounceList ) )
            {
                if ( std::sscanf( boost::get< MetaInfoString >( metaUrl ).c_str(), "udp://%1023[^:]:%hd", hostname, &port ) != 2 || hostname[ 0 ] == '\0' )
                    continue;

                bai::udp::resolver::query query( bai::udp::v4(), hostname, std::to_string( port ) );
                bai::udp::resolver resolver( ioService );
        
                try
                {
                    bai::udp::resolver::iterator end;
                    for ( auto it = resolver.resolve( query ); it != end; ++it )
                        result.emplace_back( bai::udp::endpoint( *it ) );
                }
                catch ( ... )
                {
                    // Log -> hostname / port : unreachable
                }
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

Torrent::Torrent( MetaInfoDictionary&& root )
    : root_( root )
    , announcers_( parse_endpoints_from_root_metainfo( boost::get< MetaInfoList >( root_.at( "announce-list" ) ) ) ) // also add "announce" as announce-list is optional
    , hashInfo_( utility::Sha1Encoder::instance().encode( BEncoder::encode( root_.at( "info" ) ) ) )
    , bytesToDownload_( parse_total_length( boost::get< MetaInfoDictionary >( root_.at( "info" ) ) ) )
    , pieceInfo_( bytesToDownload_,
                  boost::get< MetaInfoInteger >( boost::get< MetaInfoDictionary >( root_.at( "info" ) ).at( "piece length" ) ),
                  boost::get< MetaInfoString >( boost::get< MetaInfoDictionary >( root_.at( "info" ) ).at( "pieces" ) ) )
{
    // NOTHING
}
