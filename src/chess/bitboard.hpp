/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/bitboard.hpp
 * 
 * inline implementation of include/chess/bitboard.h
 * 
 */



/* HEADER GUARD */
#ifndef CHESS_BITBOARD_HPP_INCLUDED
#define CHESS_BITBOARD_HPP_INCLUDED



/* INCLUDES */
#include <chess/bitboard.h>



/** @name  vertical_flip
 * 
 * @brief  flip the board vertically
 * @return a new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::vertical_flip () const noexcept 
{
    /* use builtin if availible, otherwise compute manually */
#ifdef CHESS_BUILTIN_BSWAP64
    return bitboard { CHESS_BUILTIN_BSWAP64 ( bits ) }; 
#else
    constexpr unsigned long long k1 = 0x00FF00FF00FF00FF, k2 = 0x0000FFFF0000FFFF;
    unsigned long long x = bits;
    x = ( ( x >>  8 ) & k1 ) | ( ( x & k1 ) <<  8 );
    x = ( ( x >> 16 ) & k2 ) | ( ( x & k2 ) << 16 );
    x = ( ( x >> 32 )      ) | ( ( x      ) << 32 );
    return bitboard { x };
#endif
}

/** @name  horizontal_flip
 * 
 * @brief  flip the board horizontally
 * @return a new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::horizontal_flip () const noexcept
{
    /* manually compute using rotations */
    constexpr unsigned long long k1 = 0x5555555555555555, k2 = 0x3333333333333333, k4 = 0x0F0F0F0F0F0F0F0F;
    unsigned long long x = bits;
    x ^= k4 & ( x ^ std::rotl ( x, 8 ) );
    x ^= k2 & ( x ^ std::rotl ( x, 4 ) );
    x ^= k1 & ( x ^ std::rotl ( x, 2 ) );
    return bitboard { std::rotr ( x, 7 ) };
}

/** @name  pos_diag_flip
 * 
 * @brief  flip the board along y=x
 * @return a new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pos_diag_flip () const noexcept
{
    /* manually compute */
    constexpr unsigned long long k1 = 0x5500550055005500, k2 = 0x3333000033330000, k4 = 0x0F0F0F0F00000000;
    unsigned long long x = bits, t;
    t  = k4 & ( x ^ ( x << 28 ) );
    x ^=      ( t ^ ( t >> 28 ) );
    t  = k2 & ( x ^ ( x << 14 ) );
    x ^=      ( t ^ ( t >> 14 ) );
    t  = k1 & ( x ^ ( x <<  7 ) );
    x ^=      ( t ^ ( t >>  7 ) );
    return bitboard { x };
}

/** @name  neg_diag_flip
 * 
 * @brief  flip the board along y=-x
 * @return a new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::neg_diag_flip () const noexcept
{
    /* manually compute */
    constexpr unsigned long long k1 = 0xAA00AA00AA00AA00, k2 = 0xCCCC0000CCCC0000, k4 = 0xF0F0F0F00F0F0F0F;
    unsigned long long x = bits, t;
    t  =      ( x ^ ( x << 36 ) );
    x ^= k4 & ( t ^ ( x >> 36 ) );
    t  = k2 & ( x ^ ( x << 18 ) );
    x ^=      ( t ^ ( t >> 18 ) );
    t  = k1 & ( x ^ ( x <<  9 ) );
    x ^=      ( t ^ ( t >>  9 ) );
    return bitboard { x };
}

/** @name  pseudo_rotate_45_clock
 * 
 * @brief  flip the positive diagonals to ranks
 * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
 * @return a new board
 */
inline constexpr chess::bitboard chess::bitboard::pseudo_rotate_45_clock () const noexcept
{
    constexpr unsigned long long k1 = 0xAAAAAAAAAAAAAAAA, k2 = 0xCCCCCCCCCCCCCCCC, k4 = 0xF0F0F0F0F0F0F0F0;
    unsigned long long x = bits;
    x ^= k1 & ( x ^ std::rotr ( x,  8 ) );
    x ^= k2 & ( x ^ std::rotr ( x, 16 ) );
    x ^= k4 & ( x ^ std::rotr ( x, 32 ) );
    return bitboard { x };
}

/** @name  pseudo_rotate_45_aclock
 * 
 * @brief  flip the negative diagonals to ranks
 * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
 * @return a new board
 */
inline constexpr chess::bitboard chess::bitboard::pseudo_rotate_45_aclock () const noexcept
{
    constexpr unsigned long long k1 = 0x5555555555555555, k2 = 0x3333333333333333, k4 = 0x0F0F0F0F0F0F0F0F;
    unsigned long long x = bits;
    x ^= k1 & ( x ^ std::rotr ( x,  8 ) );
    x ^= k2 & ( x ^ std::rotr ( x, 16 ) );
    x ^= k4 & ( x ^ std::rotr ( x, 32 ) );
    return bitboard { x };
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_HPP_INCLUDED */