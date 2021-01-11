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

/** @name  fill_[compass]
 * 
 * @brief  Fill the board in a given direction taking into account occluders
 * @see    https://www.chessprogramming.org/Kogge-Stone_Algorithm#OccludedFill
 * @param  p: Propagator set: set bits are where the board is allowed to flow, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::fill_n ( bitboard p ) const noexcept
{
    bitboard x { bits };
    x |= p & ( x <<  8 );
    p &=     ( p <<  8 );
    x |= p & ( x << 16 );
    p &=     ( p << 16 );
    x |= p & ( x << 32 );
    return x;
}
inline constexpr chess::bitboard chess::bitboard::fill_s ( bitboard p ) const noexcept
{
    bitboard x { bits };
    x |= p & ( x >>  8 );
    p &=     ( p >>  8 );
    x |= p & ( x >> 16 );
    p &=     ( p >> 16 );
    x |= p & ( x >> 32 );
    return x;
}
inline constexpr chess::bitboard chess::bitboard::fill_e ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::not_a_file };
    bitboard x { bits };
    p &= k1;
    x |= p & ( x << 1 );
    p &=     ( p << 1 );
    x |= p & ( x << 2 );
    p &=     ( p << 2 );
    x |= p & ( x << 4 );
    return x;
}
inline constexpr chess::bitboard chess::bitboard::fill_w ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::not_a_file };
    bitboard x { bits };
    p &= k1;
    x |= p & ( x >> 1 );
    p &=     ( p >> 1 );
    x |= p & ( x >> 2 );
    p &=     ( p >> 2 );
    x |= p & ( x >> 4 );
    return  x;
}
inline constexpr chess::bitboard chess::bitboard::fill_ne ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::not_a_file };
    bitboard x { bits };
    p &= k1;
    x |= p & ( x <<  9 );
    p &=     ( p <<  9 );
    x |= p & ( x << 18 );
    p &=     ( p << 18 );
    x |= p & ( x << 36 );
    return x;
}
inline constexpr chess::bitboard chess::bitboard::fill_nw ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::not_h_file };
    bitboard x { bits };
    p &= k1;
    x |= p & ( x <<  7 );
    p &=     ( p <<  7 );
    x |= p & ( x << 14 );
    p &=     ( p << 14 );
    x |= p & ( x << 28 );
    return x;
}
inline constexpr chess::bitboard chess::bitboard::fill_se ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::not_a_file };
    bitboard x { bits };
    p &= k1;
    x |= p & ( x >>  7 );
    p &=     ( p >>  7 );
    x |= p & ( x >> 14 );
    p &=     ( p >> 14 );
    x |= p & ( x >> 28 );
    return x;
}
inline constexpr chess::bitboard chess::bitboard::fill_sw ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::not_h_file };
    bitboard x { bits };
    p &= k1;
    x |= p & ( x >>  9 );
    p &=     ( p >>  9 );
    x |= p & ( x >> 18 );
    p &=     ( p >> 18 );
    x |= p & ( x >> 36 );
    return x;
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_HPP_INCLUDED */