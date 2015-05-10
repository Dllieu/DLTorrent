//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_ROOTMETAINFODICTIONARY_H__
#define __PARSING_ROOTMETAINFODICTIONARY_H__

#include <boost/asio/ip/udp.hpp>
#include <array>

#include "MetaInfo.h"

namespace bai = boost::asio::ip;

namespace parsing
{
    class RootMetaInfo
    {
    public:
        const std::vector< bai::udp::endpoint >&    getAnnouncers() const;
        const std::array< char, 20 >&               getHashInfo() const;

        RootMetaInfo( const RootMetaInfo& ) = delete;
        RootMetaInfo& operator=( const RootMetaInfo& ) = delete;

    private:
        friend class TorrentReader;
        explicit RootMetaInfo( MetaInfoDictionary&& root );

    public:
        MetaInfoDictionary                  root_;
        std::vector< bai::udp::endpoint >   announcers_;
        std::array< char, 20 >              hashInfo_;
    };
}

#endif // ! __PARSING_ROOTMETAINFODICTIONARY_H__
