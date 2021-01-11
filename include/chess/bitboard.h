/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/bitboard.h
 * 
 * Header file for managing a chess bitboard
 * 
 */



/* HEADER GUARD */
#ifndef CHESS_BITBOARD_H_INCLUDED
#define CHESS_BITBOARD_H_INCLUDED



/* INCLUDES */
#include <bit>
#include <chess/builtin_macros.h>
#include <string>



/* DECLARATIONS */

namespace chess
{
    /* class bitboard
     *
     * Purely a 64 bit integer with member functions to aid access, query and manipulation.
     * Bitboards are little-endian rank-file bijective mappings stored in 64-bit integers.
     */
    class bitboard;
}



/* CLASS BITBOARD DEFINITION */

/* class bitboard
 * 
 * Purely a 64 bit integer with member functions to aid access, query and manipulation.
 * Bitboards are little-endian rank-file bijective mappings stored in 64-bit integers.
 */
class chess::bitboard
{
public:

    /* TYPES */

    /* struct masks
     *
     * Special masks for bitboards
     */
    struct masks
    {
        static constexpr unsigned long long empty         { 0x0000000000000000 };
        static constexpr unsigned long long universe      { 0xFFFFFFFFFFFFFFFF };
        static constexpr unsigned long long white_squares { 0x55AA55AA55AA55AA };
        static constexpr unsigned long long black_squares { 0xAA55AA55AA55AA55 };
        static constexpr unsigned long long a_file        { 0x0101010101010101 };
        static constexpr unsigned long long h_file        { 0x8080808080808080 };
        static constexpr unsigned long long not_a_file    { 0xFEFEFEFEFEFEFEFE };
        static constexpr unsigned long long not_h_file    { 0x7F7F7F7F7F7F7F7F };
    };



    /* CONSTRUCTORS */

    /** @name default constructor */
    constexpr bitboard () noexcept : bits { 0 } {}

    /** @name bits constructor */
    explicit constexpr bitboard ( unsigned long long _bits ) noexcept : bits { _bits } {}



    /* SET OPERATORS */

    /** @name  operator==, operator!=
     * 
     * @brief  Equality comparison operators
     * @param  other: The bitboard to compare
     * @return boolean
     */
    constexpr bool operator== ( bitboard other ) const noexcept { return ( bits == other.bits ); }
    constexpr bool operator!= ( bitboard other ) const noexcept { return ( bits != other.bits ); }

    /** @name  operator bool, operator!
     * 
     * @brief  Truth operators
     * @return boolean
     */
    explicit constexpr operator bool  () const noexcept { return bits; }
    constexpr bool operator! () const noexcept { return !bits; }

    /** @name  operator&, operator|, operator^
     * 
     * @brief  Bitwise operators
     * @param  other: The other bitboard required for the bitwise operation
     * @return A new bitboard from the bitwise operation of this and other
     */
    constexpr bitboard operator& ( bitboard other ) const noexcept { return bitboard { bits & other.bits }; }
    constexpr bitboard operator| ( bitboard other ) const noexcept { return bitboard { bits | other.bits }; }
    constexpr bitboard operator^ ( bitboard other ) const noexcept { return bitboard { bits ^ other.bits }; }

    /** @name  operator~
     * 
     * @brief  Ones-complement
     * @return A new bitboard from the ones-complement of this bitboard
     */
    constexpr bitboard operator~ () const noexcept { return bitboard { ~bits }; }

    /** @name  operator&=, operator|=, operator^= 
     *   
     * @brief  Inline bitwise operators
     * @param  other: The other bitboard required for the bitwise operation
     * @return A reference to this bitboard
     */
    constexpr bitboard& operator&= ( bitboard other ) noexcept { bits &= other.bits; return * this; }
    constexpr bitboard& operator|= ( bitboard other ) noexcept { bits |= other.bits; return * this; }
    constexpr bitboard& operator^= ( bitboard other ) noexcept { bits ^= other.bits; return * this; }

    /** @name  operator<<, operator>>
     * 
     * @brief  Binary shift operators
     * @param  offset: The amount to shift by
     * @return A new bitboard
     */
    constexpr bitboard operator<< ( unsigned offset ) const noexcept { return bitboard { bits << offset }; }
    constexpr bitboard operator>> ( unsigned offset ) const noexcept { return bitboard { bits >> offset }; }

    /** @name  operator<<=, operator>>=
     * 
     * @brief  Inline binary shift operators
     * @param  offset: The amount to shift by
     * @return A reference to this bitboard
     */
    constexpr bitboard& operator<<= ( unsigned offset ) noexcept { bits <<= offset; return * this; }
    constexpr bitboard& operator>>= ( unsigned offset ) noexcept { bits >>= offset; return * this; }



    /* SET OPERATIONS */

    /** @name  rel_comp
     *
     * @brief  The complement of this bitboard relative to other
     * @param  other: The other bitboard as described above
     * @return A new bitboard
     */
    constexpr bitboard rel_comp ( bitboard other ) const noexcept { return bitboard { ~bits & other.bits }; }

    /** @name  implication
     * 
     * @brief  The bitboard such that this implies other for all bits
     * @param  other: The other bitboard as described above
     * @return A new bitboard
     */
    constexpr bitboard implication ( bitboard other ) const noexcept { return bitboard { ~bits | other.bits }; }

    /** @name  xnor
     * 
     * @brief  Bitwise xnor
     * @param  other: The other bitboard to apply xnor to
     * @return A new bitboard
     */
    constexpr bitboard xnor ( bitboard other ) const noexcept { return bitboard { ~( bits ^ other.bits ) }; }

    /** @name  nand
     * 
     * @brief  Bitwise nand
     * @param  other: The other bitboard to apply nand to
     * @return A new bitboard
     */
    constexpr bitboard nand ( bitboard other ) const noexcept { return bitboard { ~( bits & other.bits ) }; }

    /** @name  is_subset
     * 
     * @brief  Finds if another bitboard is a subset of this bitboard
     * @param  other: The bitboard that is a potential subset
     * @return boolean
     */
    constexpr bool is_subset ( bitboard other ) const noexcept { return ( ( bits & other.bits ) == other.bits ); }

    /** @name  is_disjoint
     * 
     * @brief  Finds if another bitboard is disjoint from this bitboard
     * @param  other: The other bitboard that is potentially disjoint
     * @return boolean
     */
    constexpr bool is_disjoint ( bitboard other ) const noexcept { return ( ( bits && other.bits ) == 0 ); }



    /* OTHER BITWISE OPERATIONS */

    /** @name  is_empty
     *  
     * @brief  Tests if the bitboard contains no set bits
     * @return boolean
     */
    constexpr bool is_empty () const noexcept { return ( bits == 0 ); }

    /** @name  is_single
     * 
     * @brief  Tests if the bitboard contains a single set bit
     * @return boolean
     */
    constexpr bool is_single () const noexcept { return ( std::has_single_bit ( bits ) ); }

    /** @name  popcount
     * 
     * @return integer
     */
    constexpr unsigned popcount () const noexcept { return std::popcount ( bits ); }

    /** @name  hamming_dist
     * 
     * @param  other: Another bitboard
     * @return The number of different bits comparing this and other
     */
    constexpr unsigned hamming_dist ( bitboard other ) const noexcept { return std::popcount ( bits ^ other.bits ); }

    /** @name  leading/trailing_zeros
     * 
     * @return The number of leading/trailing zeros
     */
    constexpr unsigned leading_zeros  () const noexcept { return std::countl_one ( bits ); }
    constexpr unsigned trailing_zeros () const noexcept { return std::countr_zero ( bits ); }

    /** @name  bit_rotl/r
     * 
     * @brief  Applied a wrapping binary shift
     * @param  offset: The amount to shift by, defined for [1,63]
     * @return A new bitboard
     */
    constexpr bitboard bit_rotl ( unsigned offset ) const noexcept { return bitboard { std::rotl ( bits, offset ) }; }
    constexpr bitboard bit_rotr ( unsigned offset ) const noexcept { return bitboard { std::rotr ( bits, offset ) }; }

    /** @name  vertical_flip
     * 
     * @brief  Flip the bitboard vertically
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
     * @return A new bitboard
     */
    constexpr bitboard vertical_flip () const noexcept;

    /** @name  horizontal_flip
     * 
     * @brief  Flip the bitboard horizontally
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
     * @return A new bitboard
     */
    constexpr bitboard horizontal_flip () const noexcept;
    
    /** @name  pos_diag_flip
     * 
     * @brief  Flip the bitboard along y=x
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
     * @return A new bitboard
     */
    constexpr bitboard pos_diag_flip () const noexcept;

    /** @name  neg_diag_flip
     * 
     * @brief  Flip the bitboard along y=-x
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
     * @return A new bitboard
     */
    constexpr bitboard neg_diag_flip () const noexcept;

    /** @name  rotate_180
     * 
     * @brief  Rotate the representation of the bitboard 180 degrees
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Rotating
     * @return A new bitboard
     */
    constexpr bitboard rotate_180 () const noexcept { return vertical_flip ().horizontal_flip (); }

    /** @name  rotate_90_(a)clock
     * 
     * @brief  Rotate the representation of the bitboard 90 degrees (anti)clockwise
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Rotating
     * @return A new bitboard
     */
    constexpr bitboard rotate_90_clock  () const noexcept { return vertical_flip ().neg_diag_flip (); }
    constexpr bitboard rotate_90_aclock () const noexcept { return vertical_flip ().pos_diag_flip (); }

    /** @name  pseudo_rotate_45_clock
     * 
     * @brief  Flip the positive diagonals to ranks
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
     * @return A new bitboard
     */
    constexpr bitboard pseudo_rotate_45_clock () const noexcept;

    /** @name  pseudo_rotate_45_aclock
     * 
     * @brief  Flip the negative diagonals to ranks
     * @see    https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
     * @return A new bitboard
     */
    constexpr bitboard pseudo_rotate_45_aclock () const noexcept;

    /** @name  shift_[compass]
     * 
     * @brief  Shift the bitboard by one step based on a compass direction
     * @see    https://www.chessprogramming.org/General_Setwise_Operations#Shifting_Bitboards
     * @return A new bitboard
     */
    constexpr bitboard shift_n  () const noexcept { return bitboard { ( bits << 8 ) }; }
    constexpr bitboard shift_s  () const noexcept { return bitboard { ( bits >> 8 ) }; }
    constexpr bitboard shift_e  () const noexcept { return bitboard { ( bits << 1 ) & masks::not_a_file }; }
    constexpr bitboard shift_w  () const noexcept { return bitboard { ( bits >> 1 ) & masks::not_h_file }; }
    constexpr bitboard shift_ne () const noexcept { return bitboard { ( bits << 9 ) & masks::not_a_file }; }
    constexpr bitboard shift_nw () const noexcept { return bitboard { ( bits << 7 ) & masks::not_h_file }; }
    constexpr bitboard shift_se () const noexcept { return bitboard { ( bits >> 7 ) & masks::not_a_file }; }
    constexpr bitboard shift_sw () const noexcept { return bitboard { ( bits >> 9 ) & masks::not_h_file }; }



    /* FILL ALGORITHMS */

    /** @name  fill_[compass]
     * 
     * @brief  Fill the board in a given direction taking into account occluders
     * @see    https://www.chessprogramming.org/Kogge-Stone_Algorithm#OccludedFill
     * @param  p: Propagator set: set bits are where the board is allowed to flow, universe by default
     * @return A new bitboard
     */
    constexpr bitboard fill_n  ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard fill_s  ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard fill_e  ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard fill_w  ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard fill_ne ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard fill_nw ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard fill_se ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard fill_sw ( bitboard p = ~bitboard {} ) const noexcept;

    /** @name  span_[compass]
     * 
     * @brief  Gives the possible movement of sliding pieces (not including the initial position), taking into account occluders and attackable pieces
     * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture, universe by default
     * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow or capture, empty by default.
     *             Should technically be a superset of pp, however ( pp | sp ) is used rather than sp alone, so this is not strictly necessary.
     * @return A new bitboard
     */
    constexpr bitboard span_n  ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_n  ( pp ).shift_n  () & ( pp | sp ) ); }
    constexpr bitboard span_s  ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_s  ( pp ).shift_s  () & ( pp | sp ) ); }
    constexpr bitboard span_e  ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_e  ( pp ).shift_e  () & ( pp | sp ) ); }
    constexpr bitboard span_w  ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_w  ( pp ).shift_w  () & ( pp | sp ) ); }
    constexpr bitboard span_ne ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_ne ( pp ).shift_ne () & ( pp | sp ) ); }
    constexpr bitboard span_nw ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_nw ( pp ).shift_nw () & ( pp | sp ) ); }
    constexpr bitboard span_se ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_se ( pp ).shift_se () & ( pp | sp ) ); }
    constexpr bitboard span_sw ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill_sw ( pp ).shift_sw () & ( pp | sp ) ); }



    /* BIT QUERY AND MODIFICATION */

    /** @name  get_value
     * 
     * @return The value of the bitboard
     */
    constexpr unsigned long long get_value () const noexcept { return bits; }

    /** @name  set, reset, toggle
     * 
     * @brief  Inline set, unset or toggle a bit
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return void
     */
    constexpr void set    ( unsigned pos ) noexcept { bits |=  single_bitset ( pos ); }
    constexpr void reset  ( unsigned pos ) noexcept { bits &= ~single_bitset ( pos ); }
    constexpr void toggle ( unsigned pos ) noexcept { bits ^=  single_bitset ( pos ); }
    constexpr void set    ( unsigned rank, unsigned file ) noexcept { bits |=  single_bitset ( rank, file ); }
    constexpr void reset  ( unsigned rank, unsigned file ) noexcept { bits &= ~single_bitset ( rank, file ); }
    constexpr void toggle ( unsigned rank, unsigned file ) noexcept { bits ^=  single_bitset ( rank, file ); }

    /** @name  test
     * 
     * @brief  Finds if a bit is set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return boolean
     */
    constexpr bool test ( unsigned pos ) const noexcept { return ( bits & single_bitset ( pos ) ); }
    constexpr bool test ( unsigned rank, unsigned file ) const noexcept { return ( bits & single_bitset ( rank, file ) ); }



    /* FORMATTING */

    /** @name  format_board
     * 
     * @brief  Return a string containing newlines for a 8x8 representation of the board
     * @param  zero: The character to insert for 0, default .
     * @param  one:  The character to insert for 1, default #
     * @return The formatted string
     */
    std::string format_board ( char zero = '.', char one = '#' ) const;



private:

    /* ATTRIBUTES */

    /* The contents of the bitboard */
    unsigned long long bits;



    /* INTERNAL METHODS */

    /** @name  single_bitset
     *
     * @brief  Create a 64-bit unsigned with one bit set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return The 64-bit integer
     */
    constexpr unsigned long long single_bitset ( unsigned pos ) const noexcept { return ( 0x1ull << pos ); }
    constexpr unsigned long long single_bitset ( unsigned rank, unsigned file ) const noexcept { return ( 0x1ull << ( rank * 8 + file ) ); }

};



/* INCLUDE INLINE IMPLEMENTATION */
#include "../../src/chess/bitboard.hpp"



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_H_INCLUDED */