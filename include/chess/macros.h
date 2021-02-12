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



/* chess_has_builtin
 * 
 * Will check if a builtin is availible (GCC or CLANG)
 */
#ifdef __has_builtin
    #define chess_has_builtin( b ) __has_builtin ( __builtin_ ## b )
#else
    #define chess_has_builtin( b ) 0
#endif




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
#if chess_has_builtin ( popcountll ) // CHESS_GCC_VERSION ( 3, 4, 6 )
    #define CHESS_BUILTIN_POPCOUNT64( x ) __builtin_popcountll ( x )
#endif



/** @name  CHESS_BUILTIN_CLZ64/CTZ64
 * 
 * @brief  Counts the leading/trailing zeros of a 64-bit integer
 * @param  x: The integer
 * @return The number of leading/trailing zeros
 */
#if chess_has_builtin ( clzll )  //CHESS_GCC_VERSION ( 3, 4, 6 )
    #define CHESS_BUILTIN_CLZ64( x ) __builtin_clzll ( x )
#endif
#if chess_has_builtin ( ctzll )
    #define CHESS_BUILTIN_CTZ64( x ) __builtin_ctzll ( x )
#endif



/** @name  CHESS_BUILTIN_BSWAP64
 * 
 * @brief  Vertically swaps the bytes of a 64-bit integer
 * @param  x: The integer
 * @return The swapped integer
 */
#if chess_has_builtin ( bswap64 ) // CHESS_GCC_VERSION ( 3, 4, 6 )
    #define CHESS_BUILTIN_BSWAP64( x ) __builtin_bswap64 ( x )
#endif



/** @name  CHESS_BUILTIN_ROL64/ROR64
 * 
 * @brief  Rotate the bits of a 64-bit integer left or right respectively
 * @param  x: The integer
 * @param  o: The offset to rotate by
 * @return The swapped integer
 */
/*
#if chess_has_builtin ( rotateleft64 )
    #define CHESS_BUILTIN_ROL64( x, o ) __builtin_rotateleft64 ( x, o )
#endif
#if chess_has_builtin ( rotateright64 )
    #define CHESS_BUILTIN_ROR64( x, o ) __builtin_rotateright64 ( x, o )
#endif
*/



/** @name  CHESS_BUILTIN_REVERSE64
 *
 * @brief  Reverses the order of bits in a 64-bit integer
 * @param  x: The integer
 * @return The reversed integer
 */
/*
#if chess_has_builtin ( bitreverse64 ) // CHESS_GCC_VERSION ( 3, 4, 6 )
    #define CHESS_BUILTIN_REVERSE64( x ) __builtin_bitreverse64 ( x )
#endif
*/



/* HEADER GUARD */
#endif /* #ifndef CHESS_MACROS_H_INCLUDED */