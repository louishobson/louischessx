/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/bitboard.cpp
 * 
 * implementation of include/chess/bitboard.h
 * 
 */



/* INCLUDES */

#include <chess/bitboard.h>



/* BITBOARD IMPLEMENTATION */



/* mask definitions */
const chess::bitboard chess::masks::light_squares { 0x55AA55AA55AA55AA };
const chess::bitboard chess::masks::dark_squares  { 0xAA55AA55AA55AA55 };



/* popcount
 *
 * @return popcount as an integer
 */
int chess::bitboard::popcount () const noexcept
{
    /* if has builtin popcount, use that, else use bitset */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_popcountll ( bits );
    #else
        return std::bitset<64> { bits }.count ();
    #endif
}



/* leading/trailing_zeros
 *
 * @return number of leading/trailing zeros
 */
int chess::bitboard::leading_zeros () const noexcept
{
    /* if no bits are set, return 64 */
    if ( !bits ) return 64;

    /* if has builtin clz, use that, else cause an error for now */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_clzll ( bits );
    #else 
        #error "todo: implement non-builtin clz"
    #endif
}
int chess::bitboard::trailing_zeros () const noexcept
{
    /* if no bits are set, return 64 */
    if ( !bits ) return 64;

    /* if has builtin ctz, use that, else cause an error for now */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_ctzll ( bits );
    #else 
        #error "todo: implement non-builtin ctz"
    #endif
}



/* leading/trailing_zeros_nocheck
 *
 * @return number of leading/trailing zeros, but undefined if bits == 0
 */
int chess::bitboard::leading_zeros_nocheck () const noexcept
{
    /* if has builtin clz, use that, else cause an error for now */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_clzll ( bits );
    #else 
        #error "todo: implement non-builtin clz"
    #endif
}
int chess::bitboard::trailing_zeros_nocheck () const noexcept
{
    /* if has builtin ctz, use that, else cause an error for now */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_ctzll ( bits );
    #else 
        #error "todo: implement non-builtin ctz"
    #endif
}