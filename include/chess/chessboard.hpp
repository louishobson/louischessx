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
 * @return One of pcolor
 */
inline chess::pcolor chess::chessboard::find_color ( unsigned pos ) const noexcept
{
    bitboard mask { 1ull << pos };
    if ( bb ( bbtype::white ) & mask ) return pcolor::white; else
    if ( bb ( bbtype::black ) & mask ) return pcolor::black; else
    return pcolor::no_piece;
}

/** @name  find_type
 * 
 * @brief  Determines the type of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return One of ptype
 */
inline chess::ptype chess::chessboard::find_type ( unsigned pos ) const noexcept
{
    bitboard mask { 1ull << pos };
    if ( bb ( bbtype::pawn   ) & mask ) return ptype::pawn;   else
    if ( bb ( bbtype::king   ) & mask ) return ptype::king;   else
    if ( bb ( bbtype::queen  ) & mask ) return ptype::queen;  else
    if ( bb ( bbtype::bishop ) & mask ) return ptype::bishop; else
    if ( bb ( bbtype::knight ) & mask ) return ptype::knight; else
    if ( bb ( bbtype::rook   ) & mask ) return ptype::rook;   else
    return ptype::no_piece;
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_HPP_INCLUDED */