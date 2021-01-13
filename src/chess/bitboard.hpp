/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/bitboard.hpp
 * 
 * Inline implementation of include/chess/bitboard.h
 * 
 */



/* HEADER GUARD */
#ifndef CHESS_BITBOARD_HPP_INCLUDED
#define CHESS_BITBOARD_HPP_INCLUDED



/* INCLUDES */
#include <chess/bitboard.h>



/* OTHER BITWISE OPERATIONS */



/** @name  vertical_flip
 * 
 * @brief  Flip the board vertically
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::vertical_flip () const noexcept 
{
    /* Use builtin if availible, otherwise compute manually */
#ifdef CHESS_BUILTIN_BSWAP64
    return bitboard { CHESS_BUILTIN_BSWAP64 ( bits ) }; 
#else
    constexpr bitboard k1 { 0x00FF00FF00FF00FF }, k2 { 0x0000FFFF0000FFFF };
    bitboard x { bits };
    x = ( ( x >>  8 ) & k1 ) | ( ( x & k1 ) <<  8 );
    x = ( ( x >> 16 ) & k2 ) | ( ( x & k2 ) << 16 );
    x = ( ( x >> 32 )      ) | ( ( x      ) << 32 );
    return x;
#endif
}

/** @name  horizontal_flip
 * 
 * @brief  Flip the board horizontally
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::horizontal_flip () const noexcept
{
    constexpr bitboard k1 { 0x5555555555555555 }, k2 { 0x3333333333333333 }, k4 { 0x0F0F0F0F0F0F0F0F };
    bitboard x { bits };
    x ^= k4 & ( x ^ x.bit_rotl ( 8 ) );
    x ^= k2 & ( x ^ x.bit_rotl ( 4 ) );
    x ^= k1 & ( x ^ x.bit_rotl ( 2 ) );
    return x.bit_rotr ( 7 );
}

/** @name  pos_diag_flip
 * 
 * @brief  Flip the board along y=x
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pos_diag_flip () const noexcept
{
    constexpr bitboard k1 { 0x5500550055005500 }, k2 { 0x3333000033330000 }, k4 { 0x0F0F0F0F00000000 };
    bitboard x { bits }, t;
    t  = k4 & ( x ^ ( x << 28 ) );
    x ^=      ( t ^ ( t >> 28 ) );
    t  = k2 & ( x ^ ( x << 14 ) );
    x ^=      ( t ^ ( t >> 14 ) );
    t  = k1 & ( x ^ ( x <<  7 ) );
    x ^=      ( t ^ ( t >>  7 ) );
    return x;
}

/** @name  neg_diag_flip
 * 
 * @brief  Flip the board along y=-x
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::neg_diag_flip () const noexcept
{
    constexpr bitboard k1 { 0xAA00AA00AA00AA00 }, k2 { 0xCCCC0000CCCC0000 }, k4 { 0xF0F0F0F00F0F0F0F };
    bitboard x { bits }, t;
    t  =      ( x ^ ( x << 36 ) );
    x ^= k4 & ( t ^ ( x >> 36 ) );
    t  = k2 & ( x ^ ( x << 18 ) );
    x ^=      ( t ^ ( t >> 18 ) );
    t  = k1 & ( x ^ ( x <<  9 ) );
    x ^=      ( t ^ ( t >>  9 ) );
    return x;
}

/** @name  pseudo_rotate_45_clock
 * 
 * @brief  Flip the positive diagonals to ranks
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pseudo_rotate_45_clock () const noexcept
{
    constexpr bitboard k1 { 0xAAAAAAAAAAAAAAAA }, k2 { 0xCCCCCCCCCCCCCCCC }, k4 { 0xF0F0F0F0F0F0F0F0 };
    bitboard x { bits };
    x ^= k1 & ( x ^ x.bit_rotr (  8 ) );
    x ^= k2 & ( x ^ x.bit_rotr ( 16 ) );
    x ^= k4 & ( x ^ x.bit_rotr ( 32 ) );
    return x;
}

/** @name  pseudo_rotate_45_aclock
 * 
 * @brief  Flip the negative diagonals to ranks
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pseudo_rotate_45_aclock () const noexcept
{
    constexpr bitboard k1 { 0x5555555555555555 }, k2 { 0x3333333333333333 }, k4 { 0x0F0F0F0F0F0F0F0F };
    bitboard x { bits };
    x ^= k1 & ( x ^ x.bit_rotr (  8 ) );
    x ^= k2 & ( x ^ x.bit_rotr ( 16 ) );
    x ^= k4 & ( x ^ x.bit_rotr ( 32 ) );
    return x;
}



/* FILL, MOVE AND CAPTURE ALGORITHMS */



/** @name  fill
 * 
 * @brief  Fill the board in a given direction taking into account occluders
 * @see    https://www.chessprogramming.org/Kogge-Stone_Algorithm#OccludedFill
 * @param  dir: The direction to shift
 * @param  p: Propagator set: set bits are where the board is allowed to flow, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::fill ( compass dir, bitboard p ) const noexcept
{
    bitboard x { bits }; int r = shift_val ( dir );
    p &=     shift_mask ( dir );
    x |= p & x.bitshift ( r );
    p &=     p.bitshift ( r );
    x |= p & x.bitshift ( r * 2 );
    p &=     p.bitshift ( r * 2 );
    x |= p & x.bitshift ( r * 4 );
    return x;
}

/** @name  pawn_push_attack
 * 
 * @brief  Gives the span of pawn pushes or attacks, depending on the compass direction given
 * @see    https://www.chessprogramming.org/Pawn_Pushes_(Bitboards)#Push_per_Side
 * @see    https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Pawns_set-wise
 * @param  dir: The direction to push or attack in
 * @param  p: Propagator set: if pushing then set bits are empty cells; if attacking then set bits are opposing pieces; universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pawn_push_attack ( compass dir, bitboard p ) const
{
    /* if is a north push */
    if ( dir == compass::n )
    {
        constexpr bitboard k1 { masks::rank_4 };
        bitboard x { shift ( compass::n ) & p };
        return { x | ( x.shift ( compass::n ) & p & k1 ) };
    } else

    /* if is a south push */
    if ( dir == compass::s )
    {
        constexpr bitboard k1 { masks::rank_4 };
        bitboard x { shift ( compass::s ) & p };
        return { x | ( x.shift ( compass::s ) & p & k1 ) };
    } else

    /* if is an attack */
    {
        assert (( dir != compass::w && dir != compass::e ));
        return shift ( dir ) & p;
    }
}

/** @name  knight_any_attack
 * 
 * @brief  Gives the union of all knight attacks
 * @see    https://www.chessprogramming.org/Knight_Pattern#Multiple_Knight_Attacks
 * @param  p: Propagator set: set bits are empty or capturable pieces, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::knight_any_attack ( bitboard p ) const noexcept
{
    return bitboard
    {
        ( ( bits << 17 | bits >> 15 ) & ~masks::file_a ) |
        ( ( bits << 15 | bits >> 17 ) & ~masks::file_h ) |
        ( ( bits << 10 | bits >>  6 ) & ~masks::file_a & ~masks::file_b ) | 
        ( ( bits <<  6 | bits >> 10 ) & ~masks::file_g & ~masks::file_h )
    } & p;
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_HPP_INCLUDED */