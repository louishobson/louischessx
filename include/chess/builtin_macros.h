/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/builtin_macros.h
 * 
 * Checks for builtin or intrinsic functions and aliases them with macros
 * 
 */



/* HEADER GUARD */
#ifndef CHESS_BUILTIN_MACROS_H_INCLUDED
#define CHESS_BUILTIN_MACROS_H_INCLUDED



/** @name  CHESS_BUILTIN_POPCOUNT64
 * 
 * @brief  Finds the popcount of a 64-bit integer
 * @param  x: The integer
 * @return The number of set zeros
 */
#if ( __GNUC__ > 3 ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ > 4 ) ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ == 4 ) && ( __GNUC_PATCHLEVEL__ >= 6 ) )
    #define CHESS_BUILTIN_POPCOUNT64( x ) __builtin_popcountll ( x )
#endif



/** @name  CHESS_BUILTIN_CLZ64/CTZ64
 * 
 * @brief  Counts the leading/trailing zeros of a 64-bit integer
 * @param  x: The integer
 * @return The number of leading/trailing zeros
 */
#if ( __GNUC__ > 3 ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ > 4 ) ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ == 4 ) && ( __GNUC_PATCHLEVEL__ >= 6 ) )
    #define CHESS_BUILTIN_CLZ64( x ) __builtin_clzll ( x )
    #define CHESS_BUILTIN_CTZ64( x ) __builtin_ctzll ( x )
#endif



/** @name  CHESS_BUILTIN_BSWAP64
 * 
 * @brief  Vertically swaps the bytes of a 64-bit integer
 * @param  x: The integer
 * @return The swapped integer
 */
#if ( __GNUC__ > 4 ) || ( ( __GNUC__ == 4 ) && ( __GNUC_MINOR__ > 3 ) ) || ( ( __GNUC__ == 4 ) && ( __GNUC_MINOR__ == 3 ) && ( __GNUC_PATCHLEVEL__ >= 6 ) )
    #define CHESS_BUILTIN_BSWAP64( x ) __builtin_bswap64 ( x )
#endif 



/* HEADER GUARD */
#endif /* #ifndef CHESS_BUILTIN_MACROS_H_INCLUDED */