//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>
#undef BOOST_FILESYSTEM_NO_DEPRECATED
#include <fstream>

#include "TorrentReader.h"
#include "RootMetaInfo.h"
#include "BDecoder.h"

namespace bfs = boost::filesystem;
using namespace parsing;

/*static*/ RootMetaInfo   TorrentReader::read( const bfs::path& torrent )
{
    if ( ! bfs::exists( torrent ) )
        throw bfs::filesystem_error( "Invalid torrent", torrent, boost::system::error_code() );

    bfs::ifstream   stream( torrent, std::ios_base::in | std::ios_base::binary );
    stream.seekg( 0, std::ios::end );
    auto bytesToRead = static_cast< size_t >( stream.tellg() );

    std::string encodedMetaInfo;
    // use resize instead of reserve as the way we populate 'encodedMetaInfo' won't change it's size
    encodedMetaInfo.resize( bytesToRead ); 

    stream.seekg( 0, std::ios::beg );
    stream.read( &encodedMetaInfo[ 0 ], bytesToRead );

    if ( ! stream || stream.gcount() != bytesToRead )
        throw bfs::filesystem_error( "Couldn't read all the data", torrent, boost::system::error_code() );

    auto root = boost::get< MetaInfoDictionary >( BDecoder::decode( encodedMetaInfo ) );
    return RootMetaInfo( std::move( root ) );
}
