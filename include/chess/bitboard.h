/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/bitboard.h
 * 
 * header file for managing a chess bitboard
 * 
 */



/* HEADER GUARD */
#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED



/* INCLUDES */
#include <array>
#include <bitset>



/* MACROS */

/* BITBOARD_HAS_BUILTIN_BIT_OPS
 *
 * only defined if the GCC version allows for builtin popcount, clz and ctz operations
 */
#if defined __GNUC__ && ( __GNUC__ > 3 ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ > 4 ) ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ == 4 ) && ( __GNUC_PATCH_LEVEL__ >= 6 ) )
    #define BITBOARD_HAS_BUILTIN_BIT_OPS
#endif



/* DECLARATIONS */

namespace chess
{
    /* class bitboard
     *
     * purely a 64 bit integer with member functions to aid access, query and manipulation
     * bitboards are little-endian rank-file bijective mappings stored in 64-bit integers
     */
    class bitboard;

    /* namespace containing bitboard masks */
    namespace masks
    {
        extern const bitboard light_squares;
        extern const bitboard dark_squares;
    }
}



/* CLASS BITBOARD DEFINITION */

/* class bitboard
 * 
 * purely a 64 bit integer with member functions to aid access, query and manipulation
 * bitboards are little-endian rank-file bijective mappings stored in 64-bit integers
 */
class chess::bitboard
{
public:

    /* CONSTRUCTORS */

    /** @brief default constructor */
    bitboard () noexcept : bits { 0 } {}

    /** @brief bits constructor */
    bitboard ( const long long unsigned _bits ) noexcept : bits { _bits } {}



    /* OPERATORS */

    /** @name  operator==, operator!=
     * 
     * @brief  equality comparison operators
     * @param  other: the board to compare
     * @return boolean
     */
    bool operator== ( const bitboard& other ) const noexcept { return ( bits == other.bits ); }
    bool operator!= ( const bitboard& other ) const noexcept { return ( bits != other.bits ); }

    /** @name  operator bool, operator!
     * 
     * @brief  truth operators
     * @return boolean
     */
    operator bool () const noexcept { return bits; }
    bool operator! () const noexcept { return !bits; }

    /** @name  operator&, operator|, operator^
     * 
     * @brief  bitwise operators
     * @param  other: the other board required for the bitwise operation
     * @return a new board from the bitwise operation of this and other
     */
    bitboard operator& ( const bitboard& other ) const noexcept { return bitboard { bits & other.bits }; }
    bitboard operator| ( const bitboard& other ) const noexcept { return bitboard { bits | other.bits }; }
    bitboard operator^ ( const bitboard& other ) const noexcept { return bitboard { bits ^ other.bits }; }

    /** @name  operator&=, operator|=, operator^= 
     *   
     * @brief  inline bitwise operators
     * @param  other: the other board required for the bitwise operation
     * @return a reference to this board after the inline bitwise operation
     */
    bitboard& operator&= ( const bitboard& other ) noexcept { bits &= other.bits; return * this; }
    bitboard& operator|= ( const bitboard& other ) noexcept { bits |= other.bits; return * this; }
    bitboard& operator^= ( const bitboard& other ) noexcept { bits ^= other.bits; return * this; }



    /** @name  popcount
     * 
     * @return popcount as an integer
     */
    int popcount () const noexcept;

    /** @name  leading/trailing_zeros
     * 
     * @return number of leading/trailing zeros
     */
    int leading_zeros () const noexcept;
    int trailing_zeros () const noexcept;

    /** @name  leading/trailing_zeros_nocheck
     * 
     * @return number of leading/trailing zeros, or undefined if bits == 0
     */
    int leading_zeros_nocheck () const noexcept;
    int trailing_zeros_nocheck () const noexcept;



private:

    /* the contents of the bitboard */
    long long unsigned bits;

};



/* INCLUDE INLINE IMPLEMENTATION */
#include "../../src/chess/bitboard.hpp"



/* HEADER GUARD */
#endif /* #ifndef BITBOARD_H_INCLUDED */