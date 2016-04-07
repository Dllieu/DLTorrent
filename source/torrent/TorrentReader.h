//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_TORRENTREADER_H__
#define __TORRENT_TORRENTREADER_H__

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem/path.hpp>
#undef BOOST_FILESYSTEM_NO_DEPRECATED

namespace torrent
{
    class Torrent;

    class TorrentReader
    {
    public:
        static Torrent   read( const boost::filesystem::path& filepath );
    };
}

#endif // ! __TORRENT_TORRENTREADER_H__
