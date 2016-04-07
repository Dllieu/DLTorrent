//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio/ip/udp.hpp>
#pragma warning( pop )

#include <array>
#include <iostream>

#include "MetaInfo.h"
#include "PieceInfo.h"

namespace bai = boost::asio::ip;

namespace torrent
{
    class Torrent
    {
    public:
        Torrent( MetaInfoDictionary&& root );
        Torrent( Torrent&& ) = default;
        Torrent&    operator=( Torrent&& ) = default;

        // Inlines
        void        display() const
        {
            std::cout << root_ << std::endl;
        }

        const std::vector< bai::udp::endpoint >&    getAnnouncers() const
        {
            return announcers_;
        }

        const std::array< char, 20 >&       getHashInfo() const
        {
            return hashInfo_;
        }

        uint64_t        getBytesToDownload() const
        {
            return bytesToDownload_;
        }

        const PieceInfo&   getPieceInfo() const
        {
            return pieceInfo_;
        }

    private:
        const MetaInfoDictionary                  root_;

        const std::vector< bai::udp::endpoint >   announcers_;
        const std::array< char, 20 >              hashInfo_;
        const uint64_t                            bytesToDownload_;
        const PieceInfo                           pieceInfo_;
    };
}
