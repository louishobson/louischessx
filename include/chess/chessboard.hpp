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
    bitboard w_east_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne );
    bitboard w_west_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw );
    bitboard b_east_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se );
    bitboard b_west_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw );

    /* Switch depending on pc.
     * Any cell defeanded twice is safe.
     * Any cells opponent is not attacking are safe.
     * Any cells that friendly is attacking once, and opponent is not double attacking are safe.
     */
    if ( pc == pcolor::white )
        return ( w_east_attack & w_west_attack ) | ~( b_east_attack | b_west_attack ) | ( ( w_east_attack ^ w_west_attack ) & ~( b_east_attack & b_west_attack ) );
    else
        return ( b_east_attack & b_west_attack ) | ~( w_east_attack | w_west_attack ) | ( ( b_east_attack ^ b_west_attack ) & ~( w_east_attack & w_west_attack ) );
}

/** @name  pawn_rams_bb
 * 
 * @brief  Get a color's pawns which are acting as rams to opposing pawns
 * @param  pc: The color which is considered to be blocking
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_rams_bb ( pcolor pc ) const noexcept
{
    /* Switch depending on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::s );
    else
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::n );
}

/** @name  pawn_levers_e/w_bb
 * 
 * @brief  Get a color's pawns which are participating in a east/west lever
 * @param  pc: The color which is considered friendly
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_levers_e_bb ( pcolor pc ) const noexcept
{
    /* Switch depending on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::sw );
    else
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::nw );
}
inline chess::bitboard chess::chessboard::pawn_levers_w_bb ( pcolor pc ) const noexcept
{
    /* Switch depending on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::se );
    else
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::ne );
}

/** @name  pawn_inner/outer/center_levers_bb
 * 
 * @brief  Get a color's pawns which are participating in inner/outer/center levers
 * @param  pc: The color which is considered friendly
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_inner_levers_bb ( pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard abc_files { bitboard::masks::file_a | bitboard::masks::file_b | bitboard::masks::file_c }; 
    constexpr bitboard fgh_files { bitboard::masks::file_f | bitboard::masks::file_g | bitboard::masks::file_h }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & abc_files ) | ( pawn_levers_w_bb ( pc ) & fgh_files );
}
inline chess::bitboard chess::chessboard::pawn_outer_levers_bb ( pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard bcd_files { bitboard::masks::file_b | bitboard::masks::file_c | bitboard::masks::file_d }; 
    constexpr bitboard efg_files { bitboard::masks::file_e | bitboard::masks::file_f | bitboard::masks::file_g }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & efg_files ) | ( pawn_levers_w_bb ( pc ) & bcd_files );
}
inline chess::bitboard chess::chessboard::pawn_center_levers_bb ( pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard d_file { bitboard::masks::file_d }; 
    constexpr bitboard e_file { bitboard::masks::file_g }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & d_file ) | ( pawn_levers_w_bb ( pc ) & e_file );
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