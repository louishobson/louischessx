/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * include/chess/chessboard.hpp
 *
 * Inline implementation of include/chess/chessboard.h
 *
 */



/* HEADER GUARD */
#ifndef CHESS_CHESSBOARD_HPP_INCLUDED
#define CHESS_CHESSBOARD_HPP_INCLUDED



/* INCLUDES */
#include <chess/chessboard.h>



/* BOARD LOOKUP */



/** @name  find_color
 *
 * @brief  Determines the color of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return piece_type::white or piece_type::black
 */
inline chess::piece_type chess::chessboard::find_color ( unsigned pos ) const noexcept
{
    bitboard mask { 1ull << pos };
    if ( bb ( piece_type::white ) & mask ) return piece_type::white; else
    if ( bb ( piece_type::black ) & mask ) return piece_type::black; else
    return piece_type::no_piece;
}

/** @name  find_type
 * 
 * @brief  Determines the type of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return One of the types in piece_type
 */
inline chess::piece_type chess::chessboard::find_type ( unsigned pos ) const noexcept
{
    bitboard mask { 1ull << pos };
    if ( bb ( piece_type::pawn   ) & mask ) return piece_type::pawn;   else
    if ( bb ( piece_type::king   ) & mask ) return piece_type::king;   else
    if ( bb ( piece_type::queen  ) & mask ) return piece_type::queen;  else
    if ( bb ( piece_type::bishop ) & mask ) return piece_type::bishop; else
    if ( bb ( piece_type::knight ) & mask ) return piece_type::knight; else
    if ( bb ( piece_type::rook   ) & mask ) return piece_type::rook;   else
    return piece_type::no_piece;
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_HPP_INCLUDED */