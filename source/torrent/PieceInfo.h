//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TORRENT_PIECES_H__
#define __TORRENT_PIECES_H__

#include <boost/utility/string_ref.hpp>

#include "MetaInfo.h"

namespace torrent
{
    // Underlyings Pieces from a torrent
    // No runtime check for out of bound
    // TODO: cleaning: ensure numberPiece_ == piecesHash_.size() and that pieceLength != 0 without an assert
    class PieceInfo
    {
    public:
        boost::string_ref   operator[]( std::size_t index ) const
        {
            return piecesHash_[ index ];
        }

        size_t              size() const
        {
            return piecesHash_.size();
        }

    private:
        friend class Torrent;
        PieceInfo( size_t totalLength, MetaInfoInteger pieceLength, const std::string& pieces )
            : totalLength_( totalLength )
            , pieceLength_( pieceLength )

            , pieces_( pieces )
        {
            piecesHash_.reserve( pieces.size() / SHA1_LENGTH );
            auto piecesView = boost::string_ref( pieces_ );
            for ( size_t i = 0; i < pieces.size(); i += SHA1_LENGTH )
                piecesHash_.emplace_back( piecesView.substr( i, SHA1_LENGTH ) );

            // TODO: remove
            // Every piece is of equal length except for the final piece, which is irregular
            assert( pieceLength != 0 );
            auto numberPiece = static_cast< size_t >( std::ceil( ( totalLength * 1.0 ) / pieceLength ) );
            assert( piecesHash_.size() == numberPiece );
        }

    private:
        const MetaInfoInteger     totalLength_;
        const MetaInfoInteger     pieceLength_; // nominal piece size, current best-practice is to keep the piece size to 512KB or less

        // Just used as owner for piecesHash_
        const std::string                   pieces_; // Pieces store 20-byte SHA1 hashes for each piece. These hashes are used to validate every piece the client downloads.
        std::vector< boost::string_ref >    piecesHash_;
    };
}

#endif /* ! __TORRENT_PIECES_H__ */
