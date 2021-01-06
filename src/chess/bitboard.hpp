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
#ifndef BITBOARD_HPP_INCLUDED
#define BITBOARD_HPP_INCLUDED



/* INCLUDES */
#include <chess/bitboard.h>



/* BITBOARD IMPLEMENTATION */



/** @name  popcount
 * 
 * @return popcount as an integer
 */
inline int chess::bitboard::popcount () const noexcept
{
    /* if has builtin popcount, use that, else use bitset 
     * however, it seems the same assembly code is generated regardless
     */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_popcountll ( bits );
    #else
        return std::bitset<64> { bits }.count ();
    #endif
}


/** @name  leading/trailing_zeros
 * 
 * @return number of leading/trailing zeros
 */
inline int chess::bitboard::leading_zeros () const noexcept
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
inline int chess::bitboard::trailing_zeros () const noexcept
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



/** @name  leading/trailing_zeros_nocheck
 * 
 * @return number of leading/trailing zeros, or undefined if bits == 0
 */
inline int chess::bitboard::leading_zeros_nocheck () const noexcept
{
    /* if has builtin clz, use that, else cause an error for now */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_clzll ( bits );
    #else 
        #error "todo: implement non-builtin clz"
    #endif
}
inline int chess::bitboard::trailing_zeros_nocheck () const noexcept
{
    /* if has builtin ctz, use that, else cause an error for now */
    #ifdef BITBOARD_HAS_BUILTIN_BIT_OPS
        return __builtin_ctzll ( bits );
    #else 
        #error "todo: implement non-builtin ctz"
    #endif
}



/* HEADER GUARD */
#endif /* #ifndef BITBOARD_HPP_INCLUDED */