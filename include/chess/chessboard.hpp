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
inline constexpr bool chess::bool_color ( const pcolor pc ) noexcept { return static_cast<bool> ( pc ); }

/** @name  other_color
 * 
 * @brief  Take a piece color and give the other color
 * @param  pc: The piece color to switch. Undefined behavior if is no_piece.
 * @return The other color of piece
 */
inline constexpr chess::pcolor chess::other_color ( const pcolor pc ) noexcept { return static_cast<pcolor> ( !static_cast<bool> ( pc ) ); }



/* PAWN CALCULATIONS */



/** @name  pawn_safe_squares_bb
 * 
 * @brief  Get the squares, with reference to a color, such that friendly pawns defending >= opposing opposing pawns attacking
 * @param  pc: The color which is considered defeanding
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_safe_squares_bb ( const pcolor pc ) const noexcept
{
    /* Get all attcks, white and black, east and west */
    const bitboard w_east_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne );
    const bitboard w_west_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw );
    const bitboard b_east_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se );
    const bitboard b_west_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw );

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
inline chess::bitboard chess::chessboard::pawn_rams_bb ( const pcolor pc ) const noexcept
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
inline chess::bitboard chess::chessboard::pawn_levers_e_bb ( const pcolor pc ) const noexcept
{
    /* Switch depending on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::sw );
    else
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::nw );
}
inline chess::bitboard chess::chessboard::pawn_levers_w_bb ( const pcolor pc ) const noexcept
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
inline chess::bitboard chess::chessboard::pawn_inner_levers_bb ( const pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard abc_files { bitboard::masks::file_a | bitboard::masks::file_b | bitboard::masks::file_c }; 
    constexpr bitboard fgh_files { bitboard::masks::file_f | bitboard::masks::file_g | bitboard::masks::file_h }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & abc_files ) | ( pawn_levers_w_bb ( pc ) & fgh_files );
}
inline chess::bitboard chess::chessboard::pawn_outer_levers_bb ( const pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard bcd_files { bitboard::masks::file_b | bitboard::masks::file_c | bitboard::masks::file_d }; 
    constexpr bitboard efg_files { bitboard::masks::file_e | bitboard::masks::file_f | bitboard::masks::file_g }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & efg_files ) | ( pawn_levers_w_bb ( pc ) & bcd_files );
}
inline chess::bitboard chess::chessboard::pawn_center_levers_bb ( const pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard d_file { bitboard::masks::file_d }; 
    constexpr bitboard e_file { bitboard::masks::file_g }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & d_file ) | ( pawn_levers_w_bb ( pc ) & e_file );
}

/** @name  pawn_doubled_in_front_bb
 * 
 * @brief  Get a color's pawns which are directly behind a friendly pawn
 * @param  pc: The color which is considered friendly
 * @return A bitboard
 */
inline chess::bitboard chess::chessboard::pawn_doubled_in_front_bb ( const pcolor pc ) const noexcept
{
    /* Switch on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).span ( compass::s );
    else 
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).span ( compass::n );
}

/** @name  half_isolanis_bb
 * 
 * @brief  Get the half isolated pawns
 * @param  pc: The color which is considered friendly
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::half_isolanis_bb ( const pcolor pc ) const noexcept
{
    /* Return the half isolanis */
    const bitboard pawns = bb ( pc, ptype::pawn );
    return ( pawns & ~pawns.pawn_attack_fill_e () ) ^ ( pawns & ~pawns.pawn_attack_fill_w () );    
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_HPP_INCLUDED */