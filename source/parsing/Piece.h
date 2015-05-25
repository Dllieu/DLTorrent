//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __PARSING_PIECES_H__
#define __PARSING_PIECES_H__

#include <boost/utility/string_ref.hpp>

#include "MetaInfo.h"

#define SHA1_LENGTH 20

namespace parsing
{
    // N blocks == 1 Piece
    class Piece
    {
    public:
        // Get a block
        boost::string_ref   operator[]( std::size_t index ) const
        {
            // no defensive check
            auto realIndex = index * SHA1_LENGTH;
            boost::string_ref tmp = blocks_;
            return tmp.substr( realIndex, realIndex + SHA1_LENGTH );
        }

        std::size_t         block_size() const
        {
            return blocksSize_;
        }

        MetaInfoInteger         piece_size() const
        {
            return pieceSize_;
        }

    private:
        friend class RootMetaInfo;
        Piece( MetaInfoInteger pieceSize, const std::string& blocks )
            : pieceSize_( pieceSize )
            , blocks_( blocks )
            , blocksSize_( blocks.size() / SHA1_LENGTH )
        {}


        MetaInfoInteger     pieceSize_;
        // Hum..... should share the string instead as it is very big
        // tried boost::string_ref here but seems MetaInfo is returning a copy piecesFromMetaInfo
        std::string         blocks_;
        std::size_t         blocksSize_;
    };
}

#undef SHA1_LENGTH

#endif /* ! __PARSING_PIECES_H__ */
