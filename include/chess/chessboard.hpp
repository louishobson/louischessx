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



/* PIECE ENUMS */



/** @name  bool_color
 * 
 * @brief  cast a piece color to a bool
 * @param  pc: The piece color to cast. Undefined behavior if is no_piece.
 * @return bool
 */
inline constexpr bool chess::bool_color ( pcolor pc ) noexcept { return static_cast<bool> ( pc ); }

/** @name  other_color
 * 
 * @brief  Take a piece color and give the other color
 * @param  pc: The piece color to switch. Undefined behavior if is no_piece.
 * @return The other color of piece
 */
inline constexpr chess::pcolor chess::other_color ( pcolor pc ) noexcept { return static_cast<pcolor> ( !static_cast<bool> ( pc ) ); }



/* FURTHER BITBOARD MANIPULATION */



/** @name  pawn_safe_squares_bb
 * 
 * @brief  Get the squares, with reference to a color, such that friendly pawns defending >= opposing opposing pawns attacking
 * @param  pc: The color which is considered defeanding
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_safe_squares_bb ( pcolor pc ) const noexcept
{
    /* Get all attcks, white and black, east and west */
    bitboard w_e_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne );
    bitboard w_w_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw );
    bitboard b_e_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se );
    bitboard b_w_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw );

    /* Switch depending on pc.
     * Any cell defeanded twice is safe.
     * Any cells opponent is not attacking are safe.
     * Any cells that friendly is attacking once, and opponent is not double attacking are safe.
     */
    if ( pc == pcolor::white )
        return ( w_e_attack & w_w_attack ) | ~( b_e_attack | b_w_attack ) | ( ( w_e_attack ^ w_w_attack ) & ~( b_e_attack & b_w_attack ) );
    else
        return ( b_e_attack & b_w_attack ) | ~( w_e_attack | w_w_attack ) | ( ( b_e_attack ^ b_w_attack ) & ~( w_e_attack & w_w_attack ) );
}




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