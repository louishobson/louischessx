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
#include <array>
#include <bit>
#include <cassert>
#include <chess/builtin_macros.h>
#include <string>



/* DECLARATIONS */

namespace chess
{
    /* u64 typedef */
    typedef unsigned long long u64;

    /* COMPASSES */

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

    /* enum straight/diag_compass
     *
     * Enums for straight and diagonal compasses.
     * Can be cast to a normal compass.
     */
    enum straight_compass { s  = 1, w  = 3, e  = 4, n  = 6 };
    enum diagonal_compass { sw = 0, se = 2, nw = 5, ne = 7 };

    /** @name  compass_start, knight/straight/diagonal_compass_start
     * 
     * @brief  Gives the first direction in a compass (useful for iterating over a compass)
     * @return A compass
     */
    constexpr compass compass_start () noexcept;
    constexpr knight_compass knight_compass_start () noexcept;
    constexpr straight_compass straight_compass_start () noexcept;
    constexpr diagonal_compass diagonal_compass_start () noexcept;

    /** @name  compass_next
     * 
     * @brief  Gives the next direction in a compass, looping back to the start if reaching the end (useful for iterating over a compass)
     * @param  dir: The compass direction to advance.
     * @return A compass
     */
    constexpr compass compass_next ( compass dir ) noexcept;
    constexpr knight_compass compass_next ( knight_compass dir ) noexcept;
    constexpr straight_compass compass_next ( straight_compass dir ) noexcept;
    constexpr diagonal_compass compass_next ( diagonal_compass dir ) noexcept;



    /* BITBOARD */

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
    explicit constexpr bitboard ( u64 _bits ) noexcept : bits { _bits } {}



    /* SET OPERATORS */

    /** @name  operator==, operator!=
     * 
     * @brief  Equality comparison operators
     * @param  other: The bitboard to compare
     * @return boolean
     */
    constexpr bool operator== ( bitboard other ) const noexcept { return bits == other.bits; }
    constexpr bool operator!= ( bitboard other ) const noexcept { return bits != other.bits; }

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
    constexpr bool contains ( bitboard other ) const noexcept { return ( bits & other.bits ) == other.bits; }

    /** @name  is_disjoint
     * 
     * @brief  Finds if another bitboard is disjoint from this bitboard
     * @param  other: The other bitboard that is potentially disjoint
     * @return boolean
     */
    constexpr bool is_disjoint ( bitboard other ) const noexcept { return ( bits && other.bits ) == 0; }

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
     * @return The number of leading/trailing zeros, 64 if the bitboard is empty
     */
    constexpr unsigned leading_zeros  () const noexcept { return std::countl_zero ( bits ); }
    constexpr unsigned trailing_zeros () const noexcept { return std::countr_zero ( bits ); }

    /** @name  leading/trailing_zeros_nocheck
     * 
     * @return The number of leading/trailing zeros, but undefined if the bitboard is empty
     */
    constexpr unsigned leading_zeros_nocheck  () const noexcept;
    constexpr unsigned trailing_zeros_nocheck () const noexcept;

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
    constexpr bitboard shift ( compass dir )          const noexcept { return bitshift ( shift_val ( dir ) ) & shift_mask ( dir ); }
    constexpr bitboard shift ( knight_compass dir )   const noexcept { return bitshift ( shift_val ( dir ) ) & shift_mask ( dir ); }



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
     * @brief  Gives the possible movement of sliding pieces (not including the initial position), taking into account occluders and attackable pieces
     * @param  dir: The direction to fill
     * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture, universe by default
     * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow or capture, empty by default.
     *             Should technically be a superset of pp, however ( pp | sp ) is used rather than sp alone, sp can simply be the set of capturable pieces.
     * @return A new bitboard
     */
    constexpr bitboard span ( compass dir, bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return fill ( dir ).shift ( dir ) & ( pp | sp ); }

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



    /* PAWN MOVES */

    /** @name  pawn_push_n/s
     * 
     * @brief  Gives the span of pawn pushes, including double pushes
     * @see    https://www.chessprogramming.org/Pawn_Pushes_(Bitboards)#Push_per_Side
     * @param  p: Propagator set: set bits are empty cells, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_push_n ( bitboard p = ~bitboard {} ) const noexcept;
    constexpr bitboard pawn_push_s ( bitboard p = ~bitboard {} ) const noexcept;

    /** @name  pawn_attack
     * 
     * @brief  Gives the span of pawn attacks
     * @see    https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Pawns_set-wise
     * @param  dir: The direction to attack in
     * @param  p: Propagator set: set bits are opposing pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_attack ( diagonal_compass dir, bitboard p = ~bitboard {} ) const noexcept { return shift ( static_cast<compass> ( dir ) ) & p; }

    /** @name  pawn_any_attack_n/s
     * 
     * @brief  Gives the union of north/south attacks
     * @see    https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Pawns_set-wise
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_any_attack_n ( bitboard p = ~bitboard {} ) const noexcept { return ( shift ( compass::nw ) | shift ( compass::ne ) ) & p; }
    constexpr bitboard pawn_any_attack_s ( bitboard p = ~bitboard {} ) const noexcept { return ( shift ( compass::sw ) | shift ( compass::se ) ) & p; }

    /** @name  pawn_double_attack_n/s
     * 
     * @brief  Gives the intersection of north/south attacks
     * @see    https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Pawns_set-wise
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_double_attack_n ( bitboard p = ~bitboard {} ) const noexcept { return shift ( compass::nw ) & shift ( compass::ne ) & p; }
    constexpr bitboard pawn_double_attack_s ( bitboard p = ~bitboard {} ) const noexcept { return shift ( compass::sw ) & shift ( compass::se ) & p; }



    /* KING MOVES */

    /** @name  king_attack
     * @brief  See shift ()
     */
    constexpr bitboard king_attack ( compass dir, bitboard p = ~bitboard {} ) const noexcept { return shift ( dir ) & p; }

    /** @name  king_any_attack
     * 
     * @brief  Gives the union of all possible king moves
     * @see    https://www.chessprogramming.org/King_Pattern#by_Calculation
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, but which are not protected by an enemy piece, universe by default.
     *         Sometimes called a 'taboo set'.
     * @param  single: If set to true, will assume the board is a singleton, false by default.
     *         Note: setting to true using an empty bitboard will cause undefined behavior.
     * @return A new bitboard
     */
    constexpr bitboard king_any_attack ( bitboard p = ~bitboard {}, bool single = false ) const noexcept;



    /* ROOK, BISHOP AND QUEEN MOVES */

    /** @name  rook/bishop/queen_attack
     * 
     * @brief  Gives the possible movement of sliding pieces (not including the initial position), taking into account occluders and attackable pieces
     * @param  dir: The direction to fill
     * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture, universe by default
     * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow or capture, empty by default.
     *             Should technically be a superset of pp, however ( pp | sp ) is used rather than sp alone, sp can simply be the set of capturable pieces.
     * @return A new bitboard
     */
    constexpr bitboard rook_attack   ( straight_compass dir, bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return span ( static_cast<compass> ( dir ), pp, sp ); }
    constexpr bitboard bishop_attack ( diagonal_compass dir, bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return span ( static_cast<compass> ( dir ), pp, sp ); }
    constexpr bitboard queen_attack  ( compass dir,          bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return span ( dir, pp, sp ); }

    /** @name  rook/bishop/queen_any_attack
     * 
     * @brief  Gives the intersection of attacks in all directions
     * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture, universe by default
     * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow or capture, empty by default.
     *             Should technically be a superset of pp, however ( pp | sp ) is used rather than sp alone, sp can simply be the set of capturable pieces.
     * @return A new bitboard
     */
    constexpr bitboard rook_all_attack   ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept;
    constexpr bitboard bishop_all_attack ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept;
    constexpr bitboard queen_all_attack  ( bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept;



    /* KNIGHT MOVES */

    /** @name  knight_attack
     * 
     * @brief  Shift the board based on knight compass directions
     * @see    https://www.chessprogramming.org/Knight_Pattern#by_Calculation
     * @param  dir: The direction to attack in
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard knight_attack ( knight_compass dir, bitboard p = ~bitboard {} ) const noexcept { return shift ( dir ) & p; }

    /** @name  knight_any_attack
     * 
     * @brief  Gives the union of all knight attacks
     * @see    https://www.chessprogramming.org/Knight_Pattern#Multiple_Knight_Attacks
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard knight_any_attack ( bitboard p = ~bitboard {} ) const noexcept;

    /** @name  knight_mult_attack
     * 
     * @brief  Gives the set of cells attacked by more than one knight
     * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
     * @return A new bitboard
     */
    constexpr bitboard knight_mult_attack ( bitboard p = ~bitboard {} ) const noexcept;



    /* BIT QUERY AND MODIFICATION */

    /** @name  get_value
     * 
     * @return The value of the bitboard
     */
    constexpr u64 get_value () const noexcept { return bits; }

    /** @name  set, reset, toggle
     * 
     * @brief  Inline set, unset or toggle a bit
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return void
     */
    constexpr void set    ( unsigned pos ) noexcept { bits |=  singleton_bitset ( pos ); }
    constexpr void reset  ( unsigned pos ) noexcept { bits &= ~singleton_bitset ( pos ); }
    constexpr void toggle ( unsigned pos ) noexcept { bits ^=  singleton_bitset ( pos ); }
    constexpr void set    ( unsigned rank, unsigned file ) noexcept { bits |=  singleton_bitset ( rank, file ); }
    constexpr void reset  ( unsigned rank, unsigned file ) noexcept { bits &= ~singleton_bitset ( rank, file ); }
    constexpr void toggle ( unsigned rank, unsigned file ) noexcept { bits ^=  singleton_bitset ( rank, file ); }

    /** @name  test
     * 
     * @brief  Finds if a bit is set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return boolean
     */
    constexpr bool test ( unsigned pos ) const noexcept { return bits & singleton_bitset ( pos ); }
    constexpr bool test ( unsigned rank, unsigned file ) const noexcept { return bits & singleton_bitset ( rank, file ); }



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
        static constexpr u64 empty           { 0x0000000000000000 };
        static constexpr u64 universe        { 0xFFFFFFFFFFFFFFFF };
        static constexpr u64 white_squares   { 0x55AA55AA55AA55AA };
        static constexpr u64 black_squares   { 0xAA55AA55AA55AA55 };

        static constexpr u64 file_a          { 0x0101010101010101 };
        static constexpr u64 file_b          { 0x0202020202020202 };
        static constexpr u64 file_c          { 0x0404040404040404 };
        static constexpr u64 file_d          { 0x0808080808080808 };
        static constexpr u64 file_e          { 0x1010101010101010 };
        static constexpr u64 file_f          { 0x2020202020202020 };
        static constexpr u64 file_g          { 0x4040404040404040 };
        static constexpr u64 file_h          { 0x8080808080808080 };

        static constexpr u64 rank_1          { 0x00000000000000FF };
        static constexpr u64 rank_2          { 0x000000000000FF00 };
        static constexpr u64 rank_3          { 0x0000000000FF0000 };
        static constexpr u64 rank_4          { 0x00000000FF000000 };
        static constexpr u64 rank_5          { 0x000000FF00000000 };
        static constexpr u64 rank_6          { 0x0000FF0000000000 };
        static constexpr u64 rank_7          { 0x00FF000000000000 };
        static constexpr u64 rank_8          { 0xFF00000000000000 };

        static constexpr u64 shift_sw         { ~rank_8 & ~file_h };
        static constexpr u64 shift_s          { ~rank_8           };
        static constexpr u64 shift_se         { ~rank_8 & ~file_a };
        static constexpr u64 shift_w          {           ~file_h };
        static constexpr u64 shift_e          {           ~file_a };
        static constexpr u64 shift_nw         { ~rank_1 & ~file_h };
        static constexpr u64 shift_n          { ~rank_1           };
        static constexpr u64 shift_ne         { ~rank_1 & ~file_a };

        static constexpr u64 knight_shift_ssw { ~rank_8 & ~rank_7 & ~file_h           };
        static constexpr u64 knight_shift_sse { ~rank_8 & ~rank_7 & ~file_a           };
        static constexpr u64 knight_shift_sww { ~rank_8 &           ~file_h & ~file_g };
        static constexpr u64 knight_shift_see { ~rank_8 &           ~file_a & ~file_b };
        static constexpr u64 knight_shift_nww { ~rank_1 &           ~file_h & ~file_g };
        static constexpr u64 knight_shift_nee { ~rank_1 &           ~file_a & ~file_b };
        static constexpr u64 knight_shift_nnw { ~rank_1 & ~rank_2 & ~file_h           };
        static constexpr u64 knight_shift_nne { ~rank_1 & ~rank_2 & ~file_a           };
    };



    /* ATTRIBUTES */

    /* The contents of the bitboard */
    u64 bits;



    /* SHIFT CONSTEXPRS */

    /* Shift amounts */
    static constexpr std::array<int, 8> shift_vals { -9, -8, -7, -1, 1, 7, 8, 9 };

    /* Knight shift amounts */
    static constexpr std::array<int, 8> knight_shift_vals { -17, -15, -10, -6, 6, 10, 15, 17 };

    /* Masks for shifts */
    static constexpr std::array<u64, 65> shift_masks
    {
        masks::shift_sw, masks::shift_s, masks::shift_se, masks::shift_w, masks::shift_e, masks::shift_nw, masks::shift_n, masks::shift_ne
    };

    /* Masks for knight shifts */
    static constexpr std::array<u64, 65> knight_shift_masks
    {
        masks::knight_shift_ssw, masks::knight_shift_sse, masks::knight_shift_sww, masks::knight_shift_see, masks::knight_shift_nww, masks::knight_shift_nee, masks::knight_shift_nnw, masks::knight_shift_nne
    };



    /* KING AND KNIGHT LOOKUP ATTACKS */

    /* King lookup attacks (extra zero allows for ctz on an empty bitboard to be used as a position) */
    static constexpr std::array<u64, 65> king_attack_lookups
    {
        0x0000000000000302, 0x0000000000000705, 0x0000000000000e0a, 0x0000000000001c14, 0x0000000000003828, 0x0000000000007050, 0x000000000000e0a0, 0x000000000000c040,
        0x0000000000030203, 0x0000000000070507, 0x00000000000e0a0e, 0x00000000001c141c, 0x0000000000382838, 0x0000000000705070, 0x0000000000e0a0e0, 0x0000000000c040c0,
        0x0000000003020300, 0x0000000007050700, 0x000000000e0a0e00, 0x000000001c141c00, 0x0000000038283800, 0x0000000070507000, 0x00000000e0a0e000, 0x00000000c040c000,
        0x0000000302030000, 0x0000000705070000, 0x0000000e0a0e0000, 0x0000001c141c0000, 0x0000003828380000, 0x0000007050700000, 0x000000e0a0e00000, 0x000000c040c00000,
        0x0000030203000000, 0x0000070507000000, 0x00000e0a0e000000, 0x00001c141c000000, 0x0000382838000000, 0x0000705070000000, 0x0000e0a0e0000000, 0x0000c040c0000000,
        0x0003020300000000, 0x0007050700000000, 0x000e0a0e00000000, 0x001c141c00000000, 0x0038283800000000, 0x0070507000000000, 0x00e0a0e000000000, 0x00c040c000000000,
        0x0302030000000000, 0x0705070000000000, 0x0e0a0e0000000000, 0x1c141c0000000000, 0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
        0x0203000000000000, 0x0507000000000000, 0x0a0e000000000000, 0x141c000000000000, 0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000,
        0x0000000000000000
    };

    /* Knight lookup attacks (extra zero allows for ctz on an empty bitboard to be used as a position) */
    static constexpr std::array<u64, 65> knight_attack_lookups
    {
        0x0000000000020400, 0x0000000000050800, 0x00000000000a1100, 0x0000000000142200, 0x0000000000284400, 0x0000000000508800, 0x0000000000a01000, 0x0000000000402000,
        0x0000000002040004, 0x0000000005080008, 0x000000000a110011, 0x0000000014220022, 0x0000000028440044, 0x0000000050880088, 0x00000000a0100010, 0x0000000040200020,
        0x0000000204000402, 0x0000000508000805, 0x0000000a1100110a, 0x0000001422002214, 0x0000002844004428, 0x0000005088008850, 0x000000a0100010a0, 0x0000004020002040,
        0x0000020400040200, 0x0000050800080500, 0x00000a1100110a00, 0x0000142200221400, 0x0000284400442800, 0x0000508800885000, 0x0000a0100010a000, 0x0000402000204000,
        0x0002040004020000, 0x0005080008050000, 0x000a1100110a0000, 0x0014220022140000, 0x0028440044280000, 0x0050880088500000, 0x00a0100010a00000, 0x0040200020400000,
        0x0204000402000000, 0x0508000805000000, 0x0a1100110a000000, 0x1422002214000000, 0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
        0x0400040200000000, 0x0800080500000000, 0x1100110a00000000, 0x2200221400000000, 0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000,
        0x0004020000000000, 0x0008050000000000, 0x00110a0000000000, 0x0022140000000000, 0x0044280000000000, 0x0088500000000000, 0x0010a00000000000, 0x0020400000000000,
        0x0000000000000000
    };



    /* INTERNAL METHODS */

    /** @name  shift_val, shift_mask
     * 
     * @brief  Get a shift value or mask from a compass direction
     * @param  dir: Compass direction
     * @return Shift value or mask
     */
    static constexpr int shift_val ( compass dir )        noexcept { return shift_vals        [ static_cast<int> ( dir ) ]; }
    static constexpr int shift_val ( knight_compass dir ) noexcept { return knight_shift_vals [ static_cast<int> ( dir ) ]; }
    static constexpr bitboard shift_mask ( compass dir )        noexcept { return bitboard { shift_masks        [ static_cast<int> ( dir ) ] }; }
    static constexpr bitboard shift_mask ( knight_compass dir ) noexcept { return bitboard { knight_shift_masks [ static_cast<int> ( dir ) ] }; }

    /** @name  king_attack_lookup, knight_attack_lookup
     * 
     * @brief  Lookup the possible moves of single king or knight
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return bitboard
     */
    static constexpr bitboard king_attack_lookup   ( unsigned pos ) noexcept { return bitboard { king_attack_lookups   [ pos ] }; }
    static constexpr bitboard knight_attack_lookup ( unsigned pos ) noexcept { return bitboard { knight_attack_lookups [ pos ] }; }
    static constexpr bitboard king_attack_lookup   ( unsigned rank, unsigned file ) noexcept { return bitboard { king_attack_lookups   [ rank * 8 + file ] }; }
    static constexpr bitboard knight_attack_lookup ( unsigned rank, unsigned file ) noexcept { return bitboard { knight_attack_lookups [ rank * 8 + file ] }; }

    /** @name  singleton_bitset
     * 
     * @brief  Create a 64-bit integer with one bit set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return 64-bit integer
     */
    static constexpr u64 singleton_bitset ( unsigned pos ) noexcept { return 1ull << pos; }
    static constexpr u64 singleton_bitset ( unsigned rank, unsigned file ) noexcept { return 1ull << ( rank * 8 + file ); }

    /** @name  singleton_bitboard
     *
     * @brief  Create a bitboard with one bit set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return bitboard
     */
    static constexpr bitboard singleton_bitboard ( unsigned pos ) noexcept { return bitboard { singleton_bitset ( pos ) }; }
    static constexpr bitboard singleton_bitboard ( unsigned rank, unsigned file ) noexcept { return bitboard { singleton_bitset ( rank, file ) }; }

};



/* INCLUDE INLINE IMPLEMENTATION */
#include <chess/bitboard.hpp>



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_H_INCLUDED */