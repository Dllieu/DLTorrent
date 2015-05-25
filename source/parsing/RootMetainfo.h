//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_ROOTMETAINFO_H__
#define __PARSING_ROOTMETAINFO_H__

#include <boost/asio/ip/udp.hpp>
#include <array>
#include <iostream>

#include "MetaInfo.h"
#include "Piece.h"

namespace bai = boost::asio::ip;

namespace parsing
{
    class RootMetaInfo
    {
    public:
        RootMetaInfo( MetaInfoDictionary&& root );

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

        const Piece&   getPiece() const
        {
            return piece_;
        }

    private:
        const MetaInfoDictionary                  root_;

        const std::vector< bai::udp::endpoint >   announcers_;
        const std::array< char, 20 >              hashInfo_;
        const uint64_t                            bytesToDownload_;
        const Piece                               piece_;
    };
}

#endif // ! __PARSING_ROOTMETAINFO_H__
