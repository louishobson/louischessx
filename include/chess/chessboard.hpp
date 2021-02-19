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




/* HASHING FUNCTION IMPLEMENTATION */

/** @name  operator ()
 * 
 * @brief  Creates a hash for a chessboard
 * @param  cb: The chessboard to hash
 * @return The hash
 */
inline std::size_t chess::chessboard::hash::operator () ( const chessboard& cb ) const noexcept
{
    /* Simply return the occupied positions */
    return cb.bb ().get_value ();
}



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



/* ALPHA_BETA_T IMPLEMENTATION */

/* alpha_beta_t
 *
 * Return type for alpha beta search.
 * Store the leaf state and accompanying value.
 */
struct chess::chessboard::alpha_beta_t
{
    chessboard state;
    int value;
};



/* FIND COLOR AND TYPE */

/** @name  find_color
 *
 * @brief  Determines the color of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return One of pcolor
 */
inline chess::pcolor chess::chessboard::find_color ( const unsigned pos ) const noexcept
{
    const bitboard mask = singleton_bitboard ( pos );
    if ( bb ( pcolor::white ) & mask ) return pcolor::white; else
    if ( bb ( pcolor::black ) & mask ) return pcolor::black; else
    return pcolor::no_piece;
}

/** @name  find_type
 * 
 * @brief  Determines the type of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pc:  The known piece color
 * @param  pos: Board position
 * @return One of ptype
 */
inline chess::ptype chess::chessboard::find_type ( const pcolor pc, const unsigned pos ) const noexcept
{
    const bitboard mask = singleton_bitboard ( pos );
    if ( ( bb ( pc ) & mask ).is_empty () ) return ptype::no_piece;
    if ( bb ( pc, ptype::pawn   ) & mask ) return ptype::pawn;   else
    if ( bb ( pc, ptype::knight ) & mask ) return ptype::knight; else
    if ( bb ( pc, ptype::bishop ) & mask ) return ptype::bishop; else
    if ( bb ( pc, ptype::rook   ) & mask ) return ptype::rook;   else
    if ( bb ( pc, ptype::queen  ) & mask ) return ptype::queen;  else
    if ( bb ( pc, ptype::king   ) & mask ) return ptype::king;   else
    return ptype::no_piece;
}



/* ATTACK LOOKUPS */

/** @name  any_attack_lookup
 * 
 * @brief  lookup an attack set based on a type
 * @param  pt: The type of the piece
 * @param  pos: The position of the piece
 * @return A bitboard for the attack set
 */
inline constexpr chess::bitboard chess::chessboard::any_attack_lookup ( ptype pt, unsigned pos ) const noexcept
{
    /* Switch based on pt */
    switch ( pt )
    {
    case ( ptype::queen  ): return bitboard::queen_attack_lookup    ( pos );
    case ( ptype::rook   ): return bitboard::straight_attack_lookup ( pos );
    case ( ptype::bishop ): return bitboard::diagonal_attack_lookup ( pos );
    case ( ptype::knight ): return bitboard::knight_attack_lookup   ( pos );
    //case ( ptype::pawn   ): return bitboard {};
    case ( ptype::king   ): return bitboard::king_attack_lookup     ( pos ); 
    default: return bitboard {};
    }
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_HPP_INCLUDED */