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
 * TO ADD:
 * 
 * PAWN AND KNIGHT ATTACKS BY LOOKUP
 * KNIGHT FORKS?
 * GENERALISED COMPASS DIRECTIONS
 * 
 */



/* HEADER GUARD */
#ifndef CHESS_BITBOARD_H_INCLUDED
#define CHESS_BITBOARD_H_INCLUDED



/* INCLUDES */
#include <bit>
#include <cassert>
#include <chess/builtin_macros.h>
#include <string>



/* DECLARATIONS */

namespace chess
{

    /* enum compass
     *
     * Enum for compass directions
     */
    enum class compass { sw, s, se, w, e, nw, n, ne };

    /* enum knight_compass
     *
     * Enum for knight compass directions
     */
    enum class knight_compass { ssw, sse, sww, see, nww, nee, nnw, nne };

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

    /** @name  contains
     * 
     * @brief  Finds if another bitboard is a subset of this bitboard
     * @param  other: The bitboard that is a potential subset
     * @return boolean
     */
    constexpr bool contains ( bitboard other ) const noexcept { return ( ( bits & other.bits ) == other.bits ); }

    /** @name  is_disjoint
     * 
     * @brief  Finds if another bitboard is disjoint from this bitboard
     * @param  other: The other bitboard that is potentially disjoint
     * @return boolean
     */
    constexpr bool is_disjoint ( bitboard other ) const noexcept { return ( ( bits && other.bits ) == 0 ); }

    /** @name  is_empty
     *  
     * @brief  Tests if the bitboard contains no set bits
     * @return boolean
     */
    constexpr bool is_empty () const noexcept { return ( bits == 0 ); }

    /** @name  is_singleton
     * 
     * @brief  Tests if the bitboard contains a single set bit
     * @return boolean
     */
    constexpr bool is_singleton () const noexcept { return ( std::has_single_bit ( bits ) ); }



    /* OTHER BITWISE OPERATIONS */

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
     * @param  offset: The amount to shift by
     * @return A new bitboard
     */
    constexpr bitboard bit_rotl ( int offset ) const noexcept { return bitboard { std::rotl ( bits, offset ) }; }
    constexpr bitboard bit_rotr ( int offset ) const noexcept { return bitboard { std::rotr ( bits, offset ) }; }

    /** @name  bitshift
     * 
     * @brief  Generalised bitwise shift, shifting left when positive and right when negative
     * @param  offset: The amount to shift by
     * @return A new bitboard
     */
    constexpr bitboard bitshift ( int offset ) const noexcept { return bitboard { offset > 0 ? ( bits << offset ) : ( bits >> -offset ) }; }

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

    /** @name  shift
     * 
     * @brief  Shift the bitboard by one step based on a compass direction
     * @see    https://www.chessprogramming.org/General_Setwise_Operations#Shifting_Bitboards
     * @param  dir: The direction to shift
     * @return A new bitboard
     */
    constexpr bitboard shift ( compass dir ) const noexcept { return bitshift ( shift_val ( dir ) ) & shift_mask ( dir ); }

    /** @name  knight_shift
     * 
     * @brief  Shift the board based on knight compas directions
     * @see    https://www.chessprogramming.org/Knight_Pattern#by_Calculation
     * @param  dir: The direction to shift
     * @return A new bitboard
     */
    constexpr bitboard knight_shift ( knight_compass dir ) const noexcept { return bitshift ( shift_val ( dir ) ) & shift_mask ( dir ); }



    /* GENERIC FILL ALGORITHMS */

    /** @name  fill
     * 
     * @brief  Fill the board in a given direction taking into account occluders
     * @see    https://www.chessprogramming.org/Kogge-Stone_Algorithm#OccludedFill
     * @param  dir: The direction to fill
     * @param  p: Propagator set: set bits are where the board is allowed to flow, universe by default
     * @return A new bitboard
     */
    constexpr bitboard fill ( compass dir, bitboard p = ~bitboard {} ) const noexcept;

    /** @name  span
     * 
     * @brief  Gives the possible movement of slSometimesiding pieces (not including the initial position), taking into account occluders and attackable pieces
     * @param  dir: The direction to fill
     * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture, universe by default
     * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow or capture, empty by default.
     *             Should technically be a superset of pp, however ( pp | sp ) is used rather than sp alone, sp can simply be the set of capturable pieces.
     * @return A new bitboard
     */
    constexpr bitboard span ( compass dir, bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return ( fill ( dir ).shift ( dir ) & ( pp | sp ) ); }

    /** @name  flood_fill
     * 
     * @brief  Fill the board in all directions until all positions reachable from the current are found
     * @see    https://www.chessprogramming.org/King_Pattern#Flood_Fill_Algorithms
     * @param  p: Propagator set: set bits are where the board is allowed to flow.
     *         Note: a piece can move over a diagonal boundary (like it would with a diagonal fill).
     * @return A new bitboard
     */
    constexpr bitboard flood_fill ( bitboard p ) const noexcept;

    /** @name  is_connected
     * 
     * @brief  Use a flood fill algorithm to test whether a set of targets can all be reached
     * @see    https://www.chessprogramming.org/King_Pattern#Flood_Fill_Algorithms
     * @param  p: Propagator set: set bits are where the board is allowed to flow.
     *         Note: a piece can move over a diagonal boundary (like it would with a diagonal fill).
     * @param  t: Target set: if all set bits can be reached then true is returned, false otherwise
     * @return boolean
     */
    constexpr bool is_connected ( bitboard p, bitboard t ) const noexcept;



    /* PIECE-SPECIFIC ALGORITHMS */



    /** @name  pawn_push_attack
     * 
     * @brief  Gives the span of pawn pushes or attacks, depending on the compass direction given
     * @see    https://www.chessprogramming.org/Pawn_Pushes_(Bitboards)#Push_per_Side
     * @see    https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Pawns_set-wise
     * @param  dir: The direction to push or attack in
     * @param  p: Propagator set: if pushing then set bits are empty cells; if attacking then set bits are opposing pieces; universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_push_attack ( compass dir, bitboard p = ~bitboard {} ) const;

    /** @name  pawn_any_attack_n/s
     * 
     * @brief  Gives the union of north/south attacks
     * @see    https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Pawns_set-wise
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_any_attack_n ( bitboard p = ~bitboard {} ) const noexcept { return ( ( shift ( compass::nw ) | shift ( compass::ne ) ) & p ); }
    constexpr bitboard pawn_any_attack_s ( bitboard p = ~bitboard {} ) const noexcept { return ( ( shift ( compass::sw ) | shift ( compass::se ) ) & p ); }

    /** @name  pawn_double_attack_n/s
     * 
     * @brief  Gives the intersection of north/south attacks
     * @see    https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Pawns_set-wise
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_double_attack_n ( bitboard p = ~bitboard {} ) const noexcept { return ( shift ( compass::nw ) & shift ( compass::ne ) & p ); }
    constexpr bitboard pawn_double_attack_s ( bitboard p = ~bitboard {} ) const noexcept { return ( shift ( compass::sw ) & shift ( compass::se ) & p ); }

    /** @name  knight_attack
     * 
     * @brief  Shift the board based on knight compass directions
     * @see    https://www.chessprogramming.org/Knight_Pattern#by_Calculation
     * @param  dir: The direction to attack in
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard knight_attack ( knight_compass dir, bitboard p = ~bitboard {} ) const noexcept { return ( knight_shift ( dir ) & p ); }

    /** @name  knight_any_attack
     * 
     * @brief  Gives the union of all knight attacks
     * @see    https://www.chessprogramming.org/Knight_Pattern#Multiple_Knight_Attacks
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard knight_any_attack ( bitboard p = ~bitboard {} ) const noexcept;

    /** @name  king_any_attack
     * 
     * @brief  Gives the union of all possible king moves
     * @see    https://www.chessprogramming.org/King_Pattern#by_Calculation
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, but which are not protected by an enemy piece, universe by default.
     *         Sometimes called a 'taboo set'.
     * @return A new bitboard
     */
    constexpr bitboard king_any_attack ( bitboard p = ~bitboard {} ) const noexcept;



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
    constexpr void set    ( unsigned pos ) noexcept { bits |=  singular_bitset ( pos ); }
    constexpr void reset  ( unsigned pos ) noexcept { bits &= ~singular_bitset ( pos ); }
    constexpr void toggle ( unsigned pos ) noexcept { bits ^=  singular_bitset ( pos ); }
    constexpr void set    ( unsigned rank, unsigned file ) noexcept { bits |=  singular_bitset ( rank, file ); }
    constexpr void reset  ( unsigned rank, unsigned file ) noexcept { bits &= ~singular_bitset ( rank, file ); }
    constexpr void toggle ( unsigned rank, unsigned file ) noexcept { bits ^=  singular_bitset ( rank, file ); }

    /** @name  test
     * 
     * @brief  Finds if a bit is set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return boolean
     */
    constexpr bool test ( unsigned pos ) const noexcept { return ( bits & singular_bitset ( pos ) ); }
    constexpr bool test ( unsigned rank, unsigned file ) const noexcept { return ( bits & singular_bitset ( rank, file ) ); }



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

    /* TYPES */

    /* struct masks
     *
     * Special masks for bitboards
     */
    struct masks
    {
        static constexpr unsigned long long empty           { 0x0000000000000000 };
        static constexpr unsigned long long universe        { 0xFFFFFFFFFFFFFFFF };
        static constexpr unsigned long long white_squares   { 0x55AA55AA55AA55AA };
        static constexpr unsigned long long black_squares   { 0xAA55AA55AA55AA55 };

        static constexpr unsigned long long file_a          { 0x0101010101010101 };
        static constexpr unsigned long long file_b          { 0x0202020202020202 };
        static constexpr unsigned long long file_c          { 0x0404040404040404 };
        static constexpr unsigned long long file_d          { 0x0808080808080808 };
        static constexpr unsigned long long file_e          { 0x1010101010101010 };
        static constexpr unsigned long long file_f          { 0x2020202020202020 };
        static constexpr unsigned long long file_g          { 0x4040404040404040 };
        static constexpr unsigned long long file_h          { 0x8080808080808080 };

        static constexpr unsigned long long rank_1          { 0x00000000000000FF };
        static constexpr unsigned long long rank_2          { 0x000000000000FF00 };
        static constexpr unsigned long long rank_3          { 0x0000000000FF0000 };
        static constexpr unsigned long long rank_4          { 0x00000000FF000000 };
        static constexpr unsigned long long rank_5          { 0x000000FF00000000 };
        static constexpr unsigned long long rank_6          { 0x0000FF0000000000 };
        static constexpr unsigned long long rank_7          { 0x00FF000000000000 };
        static constexpr unsigned long long rank_8          { 0xFF00000000000000 };

        static constexpr unsigned long long shift_sw         { ~rank_8 & ~file_h };
        static constexpr unsigned long long shift_s          { ~rank_8           };
        static constexpr unsigned long long shift_se         { ~rank_8 & ~file_a };
        static constexpr unsigned long long shift_w          {           ~file_h };
        static constexpr unsigned long long shift_e          {           ~file_a };
        static constexpr unsigned long long shift_nw         { ~rank_1 & ~file_h };
        static constexpr unsigned long long shift_n          { ~rank_1           };
        static constexpr unsigned long long shift_ne         { ~rank_1 & ~file_a };

        static constexpr unsigned long long knight_shift_ssw { ~rank_8 & ~rank_7 & ~file_h           };
        static constexpr unsigned long long knight_shift_sse { ~rank_8 & ~rank_7 & ~file_a           };
        static constexpr unsigned long long knight_shift_sww { ~rank_8 &           ~file_h & ~file_g };
        static constexpr unsigned long long knight_shift_see { ~rank_8 &           ~file_a & ~file_b };
        static constexpr unsigned long long knight_shift_nww { ~rank_1 &           ~file_h & ~file_g };
        static constexpr unsigned long long knight_shift_nee { ~rank_1 &           ~file_a & ~file_b };
        static constexpr unsigned long long knight_shift_nnw { ~rank_1 & ~rank_2 & ~file_h           };
        static constexpr unsigned long long knight_shift_nne { ~rank_1 & ~rank_2 & ~file_a           };
    };



    /* ATTRIBUTES */

    /* The contents of the bitboard */
    unsigned long long bits;

    /* Shift amounts */
    static constexpr int shift_vals [] = { -9, -8, -7, -1, 1, 7, 8, 9 };

    /* Knight shift amounts */
    static constexpr int knight_shift_vals [] = { -17, -15, -10, -6, 6, 10, 15, 17 };

    /* Masks for shifts */
    static constexpr unsigned long long shift_masks [] =
    {
        masks::shift_sw, masks::shift_s, masks::shift_se, masks::shift_w, masks::shift_e, masks::shift_nw, masks::shift_n, masks::shift_ne
    };

    /* Masks for knight shifts */
    static constexpr unsigned long long knight_shift_masks [] =
    {
        masks::knight_shift_ssw, masks::knight_shift_sse, masks::knight_shift_sww, masks::knight_shift_see, masks::knight_shift_nww, masks::knight_shift_nee, masks::knight_shift_nnw, masks::knight_shift_nne
    };



    /* INTERNAL METHODS */

    /** @name  ctoi
     * 
     * @brief  Cast a compass direction to an integer
     * @param  dir: Compass direction
     * @return integer
     */
    static constexpr int ctoi ( compass dir ) noexcept { return static_cast<int> ( dir ); }
    static constexpr int ctoi ( knight_compass dir ) noexcept { return static_cast<int> ( dir ); }

    /** @name  shift_val, shift_mask
     * 
     * @brief  Get a shift value or mask from a compass direction
     * @param  dir: Compass direction
     * @return Shift value or mask
     */
    static constexpr int shift_val ( compass dir )        noexcept { return shift_vals         [ ctoi ( dir ) ]; }
    static constexpr int shift_val ( knight_compass dir ) noexcept { return knight_shift_vals  [ ctoi ( dir ) ]; }
    static constexpr bitboard shift_mask ( compass dir )        noexcept { return bitboard { shift_masks        [ ctoi ( dir ) ] }; }
    static constexpr bitboard shift_mask ( knight_compass dir ) noexcept { return bitboard { knight_shift_masks [ ctoi ( dir ) ] }; }

    /** @name  singular_bitset
     *
     * @brief  Create a 64-bit unsigned with one bit set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return The 64-bit integer
     */
    static constexpr unsigned long long singular_bitset ( unsigned pos ) noexcept { return ( 0x1ull << pos ); }
    static constexpr unsigned long long singular_bitset ( unsigned rank, unsigned file ) noexcept { return ( 0x1ull << ( rank * 8 + file ) ); }

};



/* INCLUDE INLINE IMPLEMENTATION */
#include "../../src/chess/bitboard.hpp"



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_H_INCLUDED */