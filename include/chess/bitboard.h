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
#ifndef CHESS_BITBOARD_H_INCLUDED
#define CHESS_BITBOARD_H_INCLUDED



/* INCLUDES */
#include <bit>
#include <byteswap.h>
#include <chess/builtin_macros.h>



/* DECLARATIONS */

namespace chess
{
    /* class bitboard
     *
     * purely a 64 bit integer with member functions to aid access, query and manipulation
     * bitboards are little-endian rank-file bijective mappings stored in 64-bit integers
     */
    class bitboard;
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

    /* TYPES */

    /* enum masks
     *
     * special masks for bitboards
     */
    enum class mask : unsigned long long
    {
        empty         = 0x0000000000000000,
        universe      = 0xFFFFFFFFFFFFFFFF,
        white_squares = 0x55AA55AA55AA55AA,
        black_squares = 0xAA55AA55AA55AA55,
        a_file        = 0x0101010101010101,
        h_file        = 0x8080808080808080,
        not_a_file    = 0xfefefefefefefefe,
        not_h_file    = 0x7f7f7f7f7f7f7f7f
    };



    /* CONSTRUCTORS */

    /** @name default constructor */
    constexpr bitboard () noexcept : bits { 0 } {}

    /** @name bits constructor */
    constexpr bitboard ( const long long unsigned _bits ) noexcept : bits { _bits } {}



    /* OPERATORS */

    /** @name  operator==, operator!=
     * 
     * @brief  equality comparison operators
     * @param  other: the board to compare
     * @return boolean
     */
    constexpr bool operator== ( const bitboard& other ) const noexcept { return ( bits == other.bits ); }
    constexpr bool operator!= ( const bitboard& other ) const noexcept { return ( bits != other.bits ); }

    /** @name  operator bool, operator!
     * 
     * @brief  truth operators
     * @return boolean
     */
    constexpr operator bool  () const noexcept { return  bits; }
    constexpr bool operator! () const noexcept { return !bits; }

    /** @name  operator&, operator|, operator^
     * 
     * @brief  bitwise operators
     * @param  other: the other board required for the bitwise operation
     * @return a new board from the bitwise operation of this and other
     */
    constexpr bitboard operator& ( const bitboard& other ) const noexcept { return bitboard { bits & other.bits }; }
    constexpr bitboard operator| ( const bitboard& other ) const noexcept { return bitboard { bits | other.bits }; }
    constexpr bitboard operator^ ( const bitboard& other ) const noexcept { return bitboard { bits ^ other.bits }; }

    /** @name  operator~
     * 
     * @brief  ones-complement
     * @return a new board from the ones-complement of this board
     */
    constexpr bitboard operator~ () const noexcept { return bitboard { ~bits }; }

    /** @name  operator&=, operator|=, operator^= 
     *   
     * @brief  inline bitwise operators
     * @param  other: the other board required for the bitwise operation
     * @return a reference to this board after the inline bitwise operation
     */
    constexpr bitboard& operator&= ( const bitboard& other ) noexcept { bits &= other.bits; return * this; }
    constexpr bitboard& operator|= ( const bitboard& other ) noexcept { bits |= other.bits; return * this; }
    constexpr bitboard& operator^= ( const bitboard& other ) noexcept { bits ^= other.bits; return * this; }



    /* OTHER BITWISE OPERATIONS */

    /** @name  rel_comp
     *
     * @brief  the complement of this board relative to other
     * @param  other: the other board as described above
     * @return a new bitboard
     */
    constexpr bitboard rel_comp ( const bitboard& other ) const noexcept { return bitboard { ~bits & other.bits }; }

    /** @name  implication
     * 
     * @brief  the board such that this implies other for all bits
     * @param  other: the other board as described above
     * @return a new board
     */
    constexpr bitboard implication ( const bitboard& other ) const noexcept { return bitboard { ~bits | other.bits }; }

    /** @name  xnor
     * 
     * @brief  bitwise xnor
     * @param  other: the other board to apply xnor to
     * @return a new board
     */
    constexpr bitboard xnor ( const bitboard& other ) const noexcept { return bitboard { ~( bits ^ other.bits ) }; }

    /** @name  nand
     * 
     * @brief  bitwise nand
     * @param  other: the other board to apply nand to
     * @return a new board
     */
    constexpr bitboard nand ( const bitboard& other ) const noexcept { return bitboard { ~( bits & other.bits ) }; }

    /** @name  is_subset
     * 
     * @brief  finds if another board is a subset of this board
     * @param  other: the bitboard that is a potential subset
     * @return boolean
     */
    constexpr bool is_subset ( const bitboard& other ) const noexcept { return ( ( bits & other.bits ) == other.bits ); }

    /** @name  is_disjoint
     * 
     * @brief  finds if another board is disjoint from this board
     * @param  other: the other bitboard that is potentially disjoint
     * @return boolean
     */
    constexpr bool is_disjoint ( const bitboard& other ) const noexcept { return ( ( bits && other.bits ) == 0 ); }

    /** @name  is_empty
     *  
     * @brief  tests if the board contains no set bits
     * @return boolean
     */
    constexpr bool is_empty () const noexcept { return ( bits == 0 ); }

    /** @name  is_single
     * 
     * @brief  tests if the board contains a single set bit
     * @return boolean
     */
    constexpr bool is_single () const noexcept { return ( std::has_single_bit ( bits ) ); }

    /** @name  popcount
     * 
     * @return popcount as an integer
     */
    constexpr unsigned popcount () const noexcept { return std::popcount ( bits ); }

    /** @name  hamming_dist
     * 
     * @param  other: another bitboard
     * @return the number of different bits comparing this and other
     */
    constexpr unsigned hamming_dist ( const bitboard& other ) const noexcept { return std::popcount ( bits ^ other.bits ); }

    /** @name  leading/trailing_zeros
     * 
     * @return number of leading/trailing zeros
     */
    constexpr unsigned leading_zeros  () const noexcept { return std::countl_one ( bits ); }
    constexpr unsigned trailing_zeros () const noexcept { return std::countr_zero ( bits ); }

    /** @name  bit_rotl/r
     * 
     * @brief  applied a wrapping binary shift
     * @param  offset: the amount to shift by, defined for [1,63]
     * @return a new bitboard
     */
    constexpr bitboard bit_rotl ( const unsigned offset ) const noexcept { return bitboard { std::rotl ( bits, offset ) }; }
    constexpr bitboard bit_rotr ( const unsigned offset ) const noexcept { return bitboard { std::rotr ( bits, offset ) }; }

    /** @name  vertical_flip
     * 
     * @brief  flip the board vertically
     * @return a new bitboard
     */
    constexpr bitboard vertical_flip () const noexcept;

    /** @name  horizontal_flip
     * 
     * @brief  flip the board horizontally
     * @return a new bitboard
     */
    constexpr bitboard horizontal_flip () const noexcept;
    
    /** @name  pos_diag_flip
     * 
     * @brief  flip the board along y=x
     * @return a new bitboard
     */
    constexpr bitboard pos_diag_flip () const noexcept;

    /** @name  neg_diag_flip
     * 
     * @brief  flip the board along y=-x
     * @return a new bitboard
     */
    constexpr bitboard neg_diag_flip () const noexcept;

    /** @name  rotate_180
     * 
     * @brief  rotate the representation of the board 180 degrees
     * @return a new board
     */
    constexpr bitboard rotate_180 () const noexcept { return vertical_flip ().horizontal_flip (); }

    /** @name  rotate_90_(a)clock
     * 
     * @brief  rotate the representation of the board 90 degrees (anti)clockwise
     * @return a new board
     */
    constexpr bitboard rotate_90_clock     () const noexcept { return vertical_flip ().neg_diag_flip (); }
    constexpr bitboard rotate_90_anticlock () const noexcept { return vertical_flip ().pos_diag_flip (); }

    /** @name  pseudo_rotate_45_clock
     * 
     * @brief  flip the positive diagonals to ranks
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
     * @return a new board
     */
    constexpr bitboard pseudo_rotate_45_clock () const noexcept;

    /** @name  pseudo_rotate_45_aclock
     * 
     * @brief  flip the negative diagonals to ranks
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
     * @return a new board
     */
    constexpr bitboard pseudo_rotate_45_aclock () const noexcept;

    /** @name  [compass]_shift
     * 
     * @brief  shift the board by one step based on a compass direction
     * @return a new board
     */
    constexpr bitboard shift_n  () const noexcept { return bitboard { bits << 8 }; }
    constexpr bitboard shift_s  () const noexcept { return bitboard { bits >> 8 }; }
    constexpr bitboard shift_e  () const noexcept { return bitboard { ( bits << 1 ) & mtoi ( mask::not_a_file ) }; }
    constexpr bitboard shift_w  () const noexcept { return bitboard { ( bits >> 1 ) & mtoi ( mask::not_h_file ) }; }
    constexpr bitboard shift_ne () const noexcept { return bitboard { ( bits << 9 ) & mtoi ( mask::not_a_file ) }; }
    constexpr bitboard shift_nw () const noexcept { return bitboard { ( bits << 7 ) & mtoi ( mask::not_h_file ) }; }
    constexpr bitboard shift_se () const noexcept { return bitboard { ( bits >> 7 ) & mtoi ( mask::not_a_file ) }; }
    constexpr bitboard shift_sw () const noexcept { return bitboard { ( bits >> 9 ) & mtoi ( mask::not_h_file ) }; }



    /* BIT QUERY AND MODIFICATION */

    /** @name  get_value
     * 
     * @return the value of the bitboard
     */
    constexpr unsigned long long get_value () const noexcept { return bits; }

    /** @name  set, reset, toggle
     * 
     * @brief  inline set, unset or toggle a bit
     * @param  pos:  the absolute position [0,63]
     * @param  rank: the rank of the bit [0,7]
     * @param  file: the file of the bit [0,7]
     * @return void
     */
    constexpr void set    ( const unsigned pos ) noexcept { bits |=  single_bitset ( pos ); }
    constexpr void reset  ( const unsigned pos ) noexcept { bits &= ~single_bitset ( pos ); }
    constexpr void toggle ( const unsigned pos ) noexcept { bits ^=  single_bitset ( pos ); }
    constexpr void set    ( const unsigned rank, const unsigned file ) noexcept { bits |=  single_bitset ( rank, file ); }
    constexpr void reset  ( const unsigned rank, const unsigned file ) noexcept { bits &= ~single_bitset ( rank, file ); }
    constexpr void toggle ( const unsigned rank, const unsigned file ) noexcept { bits ^=  single_bitset ( rank, file ); }

    /** @name  test
     * 
     * @brief  finds if a bit is set
     * @param  pos:  the absolute position [0,63]
     * @param  rank: the rank of the bit [0,7]
     * @param  file: the file of the bit [0,7]
     * @return boolean
     */
    constexpr bool test ( const unsigned pos ) const noexcept { return ( bits & single_bitset ( pos ) ); }
    constexpr bool test ( const unsigned rank, const unsigned file ) const noexcept { return ( bits & single_bitset ( rank, file ) ); }



private:

    /* ATTRIBUTES */

    /* the contents of the bitboard */
    unsigned long long bits;



    /* INTERNAL METHODS */

    /** @name  mtoi
     * 
     * @brief  converts a mask enum to a unsigned long long
     * @param  m: the mask to convert
     * @return unsigned long long
     */
    constexpr unsigned long long mtoi ( const mask m ) const noexcept { return static_cast<unsigned long long> ( m ); }

    /** @name  single_bitset
     *
     * @brief  create a 64-bit unsigned with one bit set
     * @param  pos:  the absolute position [0,63]
     * @param  rank: the rank of the bit [0,7]
     * @param  file: the file of the bit [0,7]
     * @return the 64-bit integer
     */
    constexpr unsigned long long single_bitset ( const unsigned pos ) const noexcept { return single_bitset ( pos ); }
    constexpr unsigned long long single_bitset ( const unsigned rank, const unsigned file ) const noexcept { return ( 0x1ull << ( rank * 8 + file ) ); }

};



/* INCLUDE INLINE IMPLEMENTATION */
#include "../../src/chess/bitboard.hpp"



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_H_INCLUDED */