//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_ROOTMETAINFO_H__
#define __PARSING_ROOTMETAINFO_H__

#include <boost/asio/ip/udp.hpp>
#include <array>

#include "MetaInfo.h"

namespace bai = boost::asio::ip;

namespace parsing
{
    class RootMetaInfo
    {
    public:
        RootMetaInfo( MetaInfoDictionary&& root );

        void        display() const;

        // Inlines
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

    private:
        MetaInfoDictionary                  root_;

        std::vector< bai::udp::endpoint >   announcers_;
        std::array< char, 20 >              hashInfo_;
        uint64_t                            bytesToDownload_;
    };
}

#endif // ! __PARSING_ROOTMETAINFO_H__
