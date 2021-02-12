/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/macros.h
 * 
 * Macro definitions and checks for builtin or intrinsic functions
 * 
 */



/* HEADER GUARD */
#ifndef CHESS_MACROS_H_INCLUDED
#define CHESS_MACROS_H_INCLUDED



/** @name  CHESS_GCC_VERSION
 * 
 * @brief  Evaluates to if GCC is greater or equal to the version specified
 * @param  x: Major version
 * @param  y: Minor version
 * @param  z: Patch level
 * @return boolean
 */
#define CHESS_GCC_VERSION( x, y, z ) ( ( __GNUC__ > x ) || ( ( __GNUC__ == x ) && ( __GNUC_MINOR__ > y ) ) || ( ( __GNUC__ == x ) && ( __GNUC_MINOR__ == y ) && ( __GNUC_PATCHLEVEL__ >= z ) ) )



/* chess_inline
 *
 * Force the method to be inlined
 */
#define chess_inline [[ gnu::always_inline ]]

/* chess_inline_constexpr
 *
 * Force the method to be inlined and constexpr
 */
#define chess_inline_constexpr [[ gnu::always_inline ]] constexpr

/* chess_hot
 *
 * Force a function to be flattened, marked as hot and no-inlined
 */
#define chess_hot [[ using gnu : flatten, hot, noinline ]]



/** @name  CHESS_BUILTIN_POPCOUNT64
 * 
 * @brief  Finds the popcount of a 64-bit integer
 * @param  x: The integer
 * @return The number of set zeros
 */
#if CHESS_GCC_VERSION ( 3, 4, 6 )
    #define CHESS_BUILTIN_POPCOUNT64( x ) __builtin_popcountll ( x )
#endif



/** @name  CHESS_BUILTIN_CLZ64/CTZ64
 * 
 * @brief  Counts the leading/trailing zeros of a 64-bit integer
 * @param  x: The integer
 * @return The number of leading/trailing zeros
 */
#if CHESS_GCC_VERSION ( 3, 4, 6 )
    #define CHESS_BUILTIN_CLZ64( x ) __builtin_clzll ( x )
    #define CHESS_BUILTIN_CTZ64( x ) __builtin_ctzll ( x )
#endif



/** @name  CHESS_BUILTIN_BSWAP64
 * 
 * @brief  Vertically swaps the bytes of a 64-bit integer
 * @param  x: The integer
 * @return The swapped integer
 */
#if CHESS_GCC_VERSION ( 3, 4, 6 )
    #define CHESS_BUILTIN_BSWAP64( x ) __builtin_bswap64 ( x )
#endif



/* HEADER GUARD */
#endif /* #ifndef CHESS_MACROS_H_INCLUDED */