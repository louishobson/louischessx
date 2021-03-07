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
#include <cassert>
#include <chess/macros.h>
#include <string>



/* DECLARATIONS */

namespace chess
{
    /* COMPASSES */

    /* enum compass
     *
     * Enum for compass directions
     */
    enum class compass : unsigned { sw, s, se, w, e, nw, n, ne };

    /* enum knight_compass
     *
     * Enum for knight compass directions
     */
    enum class knight_compass : unsigned { ssw, sse, sww, see, nww, nee, nnw, nne };

    /* enum straight/diag_compass
     *
     * Enums for straight and diagonal compasses.
     * Can be cast to a normal compass.
     */
    enum straight_compass : unsigned { s  = 1, w  = 3, e  = 4, n  = 6 };
    enum diagonal_compass : unsigned { sw = 0, se = 2, nw = 5, ne = 7 };

    /** @name  cast_compass
     * 
     * @brief  Cast a compass to an integer
     * @param  dir: The compass to cast
     * @return unsigned
     */
    constexpr unsigned cast_compass ( compass dir ) noexcept;
    constexpr unsigned cast_compass ( knight_compass dir ) noexcept;
    constexpr unsigned cast_compass ( straight_compass dir ) noexcept;
    constexpr unsigned cast_compass ( diagonal_compass dir ) noexcept;

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

    /* class chessboard 
     *
     * Store and manipulate a bitboard-based chess board.
     * Forward reference to bitboard can friend it.
     */
    class chessboard;



    /* BITBOARD CREATION */

    /** @name  singleton_bitboard
     * 
     * @brief  Create a singleton bitboard from a position
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return bitboard
     */
    constexpr bitboard singleton_bitboard ( unsigned pos ) noexcept;
    constexpr bitboard singleton_bitboard ( unsigned rank, unsigned file ) noexcept;
}



/* CLASS BITBOARD DEFINITION */

/* class bitboard
 * 
 * Purely a 64 bit integer with member functions to aid access, query and manipulation.
 * Bitboards are little-endian rank-file bijective mappings stored in 64-bit integers.
 */
class chess::bitboard
{

    /* Friend chessboard */
    friend chessboard;

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

    /** @name  only_if
     * 
     * @brief  If the condition is true, returns the bitboard unchanged, else returns an empty bitboard
     * @param  cond: The condition described above
     * @return A new bitboard
     */
    constexpr bitboard only_if ( bool cond ) const noexcept { return bitboard { cond * bits }; }

    /** @name  all_if
     * 
     * @brief  If the condition is true, returns a full bitboard, else returns the bitboard unchanged
     * @param  cond: The condition described above
     * @return A new bitboard
     */
    constexpr bitboard all_if ( bool cond ) const noexcept { return bitboard { bits | ( cond * ~0ull ) }; }

    /** @name  contains
     * 
     * @brief  Finds if another bitboard is a subset of this bitboard
     * @param  other: The bitboard that is a potential subset
     * @return boolean
     */
    constexpr bool contains ( bitboard other ) const noexcept { return ( bits & other.bits ) == other.bits; }

    /** @name  has_common
     * 
     * @brief  Finds if another bitboard has any common set bits
     * @param  other: The bitboard to test against
     * @return boolean
     */
    constexpr bool has_common ( bitboard other ) const noexcept { return ( bits & other.bits ) != 0; }

    /** @name  is_disjoint
     * 
     * @brief  Finds if another bitboard is disjoint from this bitboard
     * @param  other: The other bitboard that is potentially disjoint
     * @return boolean
     */
    constexpr bool is_disjoint ( bitboard other ) const noexcept { return ( bits & other.bits ) == 0; }

    /** @name  is_empty
     *  
     * @brief  Tests if the bitboard contains no set bits
     * @return boolean
     */
    constexpr bool is_empty () const noexcept { return bits == 0; }

    /** @name  is_nonempty
     * 
     * @brief  Tests if the bitboard containts set bits
     * @return boolean
     */
    constexpr bool is_nonempty () const noexcept { return bits != 0; }

    /** @name  is_singleton
     * 
     * @brief  Tests if the bitboard contains a single set bit
     * @return boolean
     */
    constexpr bool is_singleton () const noexcept;



    /* OTHER BITWISE OPERATIONS */

    /** @name  popcount
     * 
     * @return integer
     */
    constexpr int popcount () const noexcept;

    /** @name  hamming_dist
     * 
     * @param  other: Another bitboard
     * @return The number of different bits comparing this and other
     */
    constexpr int hamming_dist ( bitboard other ) const noexcept;

    /** @name  leading/trailing_zeros
     * 
     * @return The number of leading/trailing zeros, 64 if the bitboard is empty
     */
    constexpr int leading_zeros  () const noexcept { return std::countl_zero ( bits ); }
    constexpr int trailing_zeros () const noexcept { return std::countr_zero ( bits ); }

    /** @name  leading/trailing_zeros_nocheck
     * 
     * @return The number of leading/trailing zeros, but undefined if the bitboard is empty
     */
    constexpr int leading_zeros_nocheck  () const noexcept;
    constexpr int trailing_zeros_nocheck () const noexcept;

    /** @name  bit_rotl/r
     * 
     * @brief  Applied a wrapping binary shift
     * @param  offset: The amount to shift by
     * @return A new bitboard
     */
    constexpr bitboard bit_rotl ( int offset ) const noexcept;
    constexpr bitboard bit_rotr ( int offset ) const noexcept;

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
    constexpr bitboard shift ( compass dir )        const noexcept { return bitshift ( shift_val ( dir ) ) & shift_mask ( dir ); }
    constexpr bitboard shift ( knight_compass dir ) const noexcept { return bitshift ( shift_val ( dir ) ) & shift_mask ( dir ); }



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
    constexpr bitboard span ( compass dir, bitboard pp = ~bitboard {}, bitboard sp = bitboard {} ) const noexcept { return fill ( dir, pp ).shift ( dir ) & ( pp | sp ); }

    /** @name  flood_fill
     * 
     * @brief  Fill the board in all directions until all positions reachable from the current are found
     * @see    https://www.chessprogramming.org/King_Pattern#Flood_Fill_Algorithms
     * @param  p: Propagator set: set bits are where the board is allowed to flow.
     *         Note: a piece can move over a diagonal boundary (like it would with a diagonal fill).
     * @return A new bitboard
     */
    constexpr bitboard flood_fill ( bitboard p ) const noexcept;

    /** @name  straight/diagonal_flood_fill
     * 
     * @brief  Similar to flood_fill, but only fill in straight or diagonal steps
     * @param  p: Propagator set: set bits are where the board is allowed to flow
     * @return A new bitboard
     */
    constexpr bitboard straight_flood_fill ( bitboard p ) const noexcept;
    constexpr bitboard diagonal_flood_fill ( bitboard p ) const noexcept;

    /** @name  flood_span
     * 
     * @brief  Fill the board in all directions until all positions reachable from the current are found.
     *         Unlike flood_fill, this method uses a primary and secondary propagator.
     * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture
     * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow with capture, empty by default
     * @return A new bitboard
     */
    constexpr bitboard flood_span ( bitboard pp, bitboard sp = bitboard {} ) const noexcept;

    /** @name  straight/diagonal_flood_span
     * 
     * @brief  Similar to flood_span, but only fill in straight or diagonal steps
     * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture
     * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow with capture, empty by default
     * @return A new bitboard
     */
    constexpr bitboard straight_flood_span ( bitboard pp, bitboard sp = bitboard {} ) const noexcept;
    constexpr bitboard diagonal_flood_span ( bitboard pp, bitboard sp = bitboard {} ) const noexcept;

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

    /** @name  file_fill
     * 
     * @brief  Fill the board north and south
     * @see    https://www.chessprogramming.org/Pawn_Fills#Filefill
     * @param  p: Propagator set: set bits are where the board is allowed to flow, universe by default
     * @return A bitboard
     */
    constexpr bitboard file_fill ( bitboard p = ~bitboard {} ) const noexcept { return fill ( compass::n, p ) | fill ( compass::s, p ); }



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

    /** @name  pawn_attack_fill_e/w
     * 
     * @brief  Give the file fill of pawn attacks
     * @param  p: Propagator set: set bits are empty cells, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_attack_fill_e ( bitboard p = ~bitboard {} ) const noexcept { return shift ( compass::e ).file_fill ( p ); }
    constexpr bitboard pawn_attack_fill_w ( bitboard p = ~bitboard {} ) const noexcept { return shift ( compass::w ).file_fill ( p ); }

    /** @name  pawn_any_attack_fill
     * 
     * @brief  Give the file fill of any pawn attack
     * @param  p: Propagator set: set bits are empty cells, universe by default
     * @return A new bitboard
     */
    constexpr bitboard pawn_any_attack_fill ( bitboard p = ~bitboard {} ) const noexcept { return ( shift ( compass::e ) | shift ( compass::w ) ).file_fill ( p ); }  



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
    constexpr unsigned long long get_value () const noexcept { return bits; }

    /** @name  set_value
     * 
     * @param  val: The value to set the bitboard to
     * @return void
     */
    constexpr void set_value ( unsigned long long val ) noexcept { bits = val; }

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

    /** @name  set_if, reset_if, toggle_if
     * 
     * @brief  Inline set, unset or toggle a bit based on a condition
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @param  cond: The condition under which the change should be made
     * @return void
     */
    constexpr void set_if    ( unsigned pos, bool cond ) noexcept { bits |=    singleton_bitset ( pos ) * cond;   }
    constexpr void reset_if  ( unsigned pos, bool cond ) noexcept { bits &= ~( singleton_bitset ( pos ) * cond ); }
    constexpr void toggle_if ( unsigned pos, bool cond ) noexcept { bits ^=    singleton_bitset ( pos ) * cond;   }
    constexpr void set_if    ( unsigned rank, unsigned file, bool cond ) noexcept { bits |=    singleton_bitset ( rank, file ) * cond;   }
    constexpr void reset_if  ( unsigned rank, unsigned file, bool cond ) noexcept { bits &= ~( singleton_bitset ( rank, file ) * cond ); }
    constexpr void toggle_if ( unsigned rank, unsigned file, bool cond ) noexcept { bits ^=    singleton_bitset ( rank, file ) * cond;   }

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

    /** @name  empty
     * 
     * @brief  Empties the bitboard
     * @return void
     */
    constexpr void empty () noexcept { bits = 0; }



    /* LOOKUPS */

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

    /** @name  straight_attack_lookup, diagonal_attack_lookup, queen_attack_lookup
     * 
     * @brief  Lookup the possible moves of single sliding piece
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return bitboard
     */
    static constexpr bitboard straight_attack_lookup ( unsigned pos ) noexcept { return bitboard { straight_attack_lookups [ pos ] }; }
    static constexpr bitboard diagonal_attack_lookup ( unsigned pos ) noexcept { return bitboard { diagonal_attack_lookups [ pos ] }; }
    static constexpr bitboard queen_attack_lookup    ( unsigned pos ) noexcept { return bitboard { queen_attack_lookups    [ pos ] }; }
    static constexpr bitboard straight_attack_lookup ( unsigned rank, unsigned file ) noexcept { return bitboard { straight_attack_lookups [ rank * 8 + file ] }; }
    static constexpr bitboard diagonal_attack_lookup ( unsigned rank, unsigned file ) noexcept { return bitboard { diagonal_attack_lookups [ rank * 8 + file ] }; }
    static constexpr bitboard queen_attack_lookup    ( unsigned rank, unsigned file ) noexcept { return bitboard { queen_attack_lookups    [ rank * 8 + file ] }; }

    /** @name  omnidir_attack_lookup
     * 
     * @brief  Lookup the possible moves of a single sliding piece in a single direction
     * @param  dir: The direction to move in
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return bitboard
     */
    static constexpr bitboard omnidir_attack_lookup ( compass dir, unsigned pos ) noexcept { return bitboard { omnidir_attack_lookups [ cast_compass ( dir ) ] [ pos ] }; }
    static constexpr bitboard omnidir_attack_lookup ( compass dir, unsigned rank, unsigned file ) noexcept { return bitboard { omnidir_attack_lookups [ cast_compass ( dir ) ] [ rank * 8 + file ] }; }



    /* FORMATTING */

    /** @name  format_board
     * 
     * @brief  Return a string containing newlines for a 8x8 representation of the board
     * @param  zero: The character to insert for 0, default .
     * @param  one:  The character to insert for 1, default #
     * @return The formatted string
     */
    std::string format_board ( char zero = '.', char one = '#' ) const;

    /** @name  name_cell
     * 
     * @brief  Get the name of the cell at a position
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return string
     */
    static std::string name_cell ( unsigned pos ) { return name_cell ( pos / 8, pos % 8 ); }
    static std::string name_cell ( unsigned rank, unsigned file ) { return std::string { { static_cast<char> ( 'a' + file ), static_cast<char> ( '1' + rank ) } }; };



//private:

    /* TYPES */

    /* struct masks
     *
     * Special masks for bitboards
     */
    struct masks
    {
        static constexpr unsigned long long empty           { 0x0000000000000000 };
        static constexpr unsigned long long universe        { 0xffffffffffffffff };
        static constexpr unsigned long long white_squares   { 0x55aa55aa55aa55aa };
        static constexpr unsigned long long black_squares   { 0xaa55aa55aa55aa55 };
        static constexpr unsigned long long center_squares  { 0x0000001818000000 };

        static constexpr unsigned long long kingside_castle_empty_squares  { 0x6000000000000060 };
        static constexpr unsigned long long queenside_castle_empty_squares { 0x0e0000000000000e };
        static constexpr unsigned long long kingside_castle_safe_squares   { 0x7000000000000070 };
        static constexpr unsigned long long queenside_castle_safe_squares  { 0x1c0000000000001c };

        static constexpr unsigned long long file_a { 0x0101010101010101 };
        static constexpr unsigned long long file_b { 0x0202020202020202 };
        static constexpr unsigned long long file_c { 0x0404040404040404 };
        static constexpr unsigned long long file_d { 0x0808080808080808 };
        static constexpr unsigned long long file_e { 0x1010101010101010 };
        static constexpr unsigned long long file_f { 0x2020202020202020 };
        static constexpr unsigned long long file_g { 0x4040404040404040 };
        static constexpr unsigned long long file_h { 0x8080808080808080 };

        static constexpr unsigned long long rank_1 { 0x00000000000000ff };
        static constexpr unsigned long long rank_2 { 0x000000000000ff00 };
        static constexpr unsigned long long rank_3 { 0x0000000000ff0000 };
        static constexpr unsigned long long rank_4 { 0x00000000ff000000 };
        static constexpr unsigned long long rank_5 { 0x000000ff00000000 };
        static constexpr unsigned long long rank_6 { 0x0000ff0000000000 };
        static constexpr unsigned long long rank_7 { 0x00ff000000000000 };
        static constexpr unsigned long long rank_8 { 0xff00000000000000 };

        static constexpr unsigned long long shift_sw { ~rank_8 & ~file_h };
        static constexpr unsigned long long shift_s  {      universe     };
        static constexpr unsigned long long shift_se { ~rank_8 & ~file_a };
        static constexpr unsigned long long shift_w  {           ~file_h };
        static constexpr unsigned long long shift_e  {           ~file_a };
        static constexpr unsigned long long shift_nw { ~rank_1 & ~file_h };
        static constexpr unsigned long long shift_n  {      universe     };
        static constexpr unsigned long long shift_ne { ~rank_1 & ~file_a };

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



    /* SHIFT CONSTEXPRS */

    /* Shift amounts */
    static constexpr int shift_vals [ 8 ] = { -9, -8, -7, -1, 1, 7, 8, 9 };

    /* Knight shift amounts */
    static constexpr int knight_shift_vals [ 8 ] = { -17, -15, -10, -6, 6, 10, 15, 17 };

    /* Masks for shifts */
    static constexpr unsigned long long shift_masks [ 8 ] =
    {
        masks::shift_sw, masks::shift_s, masks::shift_se, masks::shift_w, masks::shift_e, masks::shift_nw, masks::shift_n, masks::shift_ne
    };

    /* Masks for knight shifts */
    static constexpr unsigned long long knight_shift_masks [ 8 ] =
    {
        masks::knight_shift_ssw, masks::knight_shift_sse, masks::knight_shift_sww, masks::knight_shift_see, masks::knight_shift_nww, masks::knight_shift_nee, masks::knight_shift_nnw, masks::knight_shift_nne
    };



    /* KING AND KNIGHT LOOKUP ATTACKS */

    /* King lookup attacks */
    static constexpr unsigned long long king_attack_lookups [ 64 ] =
    {
        0x0000000000000302, 0x0000000000000705, 0x0000000000000e0a, 0x0000000000001c14, 0x0000000000003828, 0x0000000000007050, 0x000000000000e0a0, 0x000000000000c040,
        0x0000000000030203, 0x0000000000070507, 0x00000000000e0a0e, 0x00000000001c141c, 0x0000000000382838, 0x0000000000705070, 0x0000000000e0a0e0, 0x0000000000c040c0,
        0x0000000003020300, 0x0000000007050700, 0x000000000e0a0e00, 0x000000001c141c00, 0x0000000038283800, 0x0000000070507000, 0x00000000e0a0e000, 0x00000000c040c000,
        0x0000000302030000, 0x0000000705070000, 0x0000000e0a0e0000, 0x0000001c141c0000, 0x0000003828380000, 0x0000007050700000, 0x000000e0a0e00000, 0x000000c040c00000,
        0x0000030203000000, 0x0000070507000000, 0x00000e0a0e000000, 0x00001c141c000000, 0x0000382838000000, 0x0000705070000000, 0x0000e0a0e0000000, 0x0000c040c0000000,
        0x0003020300000000, 0x0007050700000000, 0x000e0a0e00000000, 0x001c141c00000000, 0x0038283800000000, 0x0070507000000000, 0x00e0a0e000000000, 0x00c040c000000000,
        0x0302030000000000, 0x0705070000000000, 0x0e0a0e0000000000, 0x1c141c0000000000, 0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
        0x0203000000000000, 0x0507000000000000, 0x0a0e000000000000, 0x141c000000000000, 0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000
    };

    /* Knight lookup attacks */
    static constexpr unsigned long long knight_attack_lookups [ 64 ] =
    {
        0x0000000000020400, 0x0000000000050800, 0x00000000000a1100, 0x0000000000142200, 0x0000000000284400, 0x0000000000508800, 0x0000000000a01000, 0x0000000000402000,
        0x0000000002040004, 0x0000000005080008, 0x000000000a110011, 0x0000000014220022, 0x0000000028440044, 0x0000000050880088, 0x00000000a0100010, 0x0000000040200020,
        0x0000000204000402, 0x0000000508000805, 0x0000000a1100110a, 0x0000001422002214, 0x0000002844004428, 0x0000005088008850, 0x000000a0100010a0, 0x0000004020002040,
        0x0000020400040200, 0x0000050800080500, 0x00000a1100110a00, 0x0000142200221400, 0x0000284400442800, 0x0000508800885000, 0x0000a0100010a000, 0x0000402000204000,
        0x0002040004020000, 0x0005080008050000, 0x000a1100110a0000, 0x0014220022140000, 0x0028440044280000, 0x0050880088500000, 0x00a0100010a00000, 0x0040200020400000,
        0x0204000402000000, 0x0508000805000000, 0x0a1100110a000000, 0x1422002214000000, 0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
        0x0400040200000000, 0x0800080500000000, 0x1100110a00000000, 0x2200221400000000, 0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000,
        0x0004020000000000, 0x0008050000000000, 0x00110a0000000000, 0x0022140000000000, 0x0044280000000000, 0x0088500000000000, 0x0010a00000000000, 0x0020400000000000
    };



    /* SLIDING LOOKUP ATTACKS */

    /* Straight lookup attacks */
    static constexpr unsigned long long straight_attack_lookups [ 64 ] =
    {
        0x01010101010101fe, 0x02020202020202fd, 0x04040404040404fb, 0x08080808080808f7, 0x10101010101010ef, 0x20202020202020df, 0x40404040404040bf, 0x808080808080807f,
        0x010101010101fe01, 0x020202020202fd02, 0x040404040404fb04, 0x080808080808f708, 0x101010101010ef10, 0x202020202020df20, 0x404040404040bf40, 0x8080808080807f80,
        0x0101010101fe0101, 0x0202020202fd0202, 0x0404040404fb0404, 0x0808080808f70808, 0x1010101010ef1010, 0x2020202020df2020, 0x4040404040bf4040, 0x80808080807f8080,
        0x01010101fe010101, 0x02020202fd020202, 0x04040404fb040404, 0x08080808f7080808, 0x10101010ef101010, 0x20202020df202020, 0x40404040bf404040, 0x808080807f808080,
        0x010101fe01010101, 0x020202fd02020202, 0x040404fb04040404, 0x080808f708080808, 0x101010ef10101010, 0x202020df20202020, 0x404040bf40404040, 0x8080807f80808080,
        0x0101fe0101010101, 0x0202fd0202020202, 0x0404fb0404040404, 0x0808f70808080808, 0x1010ef1010101010, 0x2020df2020202020, 0x4040bf4040404040, 0x80807f8080808080,
        0x01fe010101010101, 0x02fd020202020202, 0x04fb040404040404, 0x08f7080808080808, 0x10ef101010101010, 0x20df202020202020, 0x40bf404040404040, 0x807f808080808080,
        0xfe01010101010101, 0xfd02020202020202, 0xfb04040404040404, 0xf708080808080808, 0xef10101010101010, 0xdf20202020202020, 0xbf40404040404040, 0x7f80808080808080
    };

    /* Diagonal lookup attacks */
    static constexpr unsigned long long diagonal_attack_lookups [ 64 ] =
    {
        0x8040201008040200, 0x0080402010080500, 0x0000804020110a00, 0x0000008041221400, 0x0000000182442800, 0x0000010204885000, 0x000102040810a000, 0x0102040810204000,
        0x4020100804020002, 0x8040201008050005, 0x00804020110a000a, 0x0000804122140014, 0x0000018244280028, 0x0001020488500050, 0x0102040810a000a0, 0x0204081020400040,
        0x2010080402000204, 0x4020100805000508, 0x804020110a000a11, 0x0080412214001422, 0x0001824428002844, 0x0102048850005088, 0x02040810a000a010, 0x0408102040004020,
        0x1008040200020408, 0x2010080500050810, 0x4020110a000a1120, 0x8041221400142241, 0x0182442800284482, 0x0204885000508804, 0x040810a000a01008, 0x0810204000402010,
        0x0804020002040810, 0x1008050005081020, 0x20110a000a112040, 0x4122140014224180, 0x8244280028448201, 0x0488500050880402, 0x0810a000a0100804, 0x1020400040201008,
        0x0402000204081020, 0x0805000508102040, 0x110a000a11204080, 0x2214001422418000, 0x4428002844820100, 0x8850005088040201, 0x10a000a010080402, 0x2040004020100804,
        0x0200020408102040, 0x0500050810204080, 0x0a000a1120408000, 0x1400142241800000, 0x2800284482010000, 0x5000508804020100, 0xa000a01008040201, 0x4000402010080402,
        0x0002040810204080, 0x0005081020408000, 0x000a112040800000, 0x0014224180000000, 0x0028448201000000, 0x0050880402010000, 0x00a0100804020100, 0x0040201008040201
    };

    /* Queen lookup attacks */
    static constexpr unsigned long long queen_attack_lookups [ 64 ] = 
    {
        0x81412111090503fe, 0x02824222120a07fd, 0x0404844424150efb, 0x08080888492a1cf7, 0x10101011925438ef, 0x2020212224a870df, 0x404142444850e0bf, 0x8182848890a0c07f,
        0x412111090503fe03, 0x824222120a07fd07, 0x04844424150efb0e, 0x080888492a1cf71c, 0x101011925438ef38, 0x20212224a870df70, 0x4142444850e0bfe0, 0x82848890a0c07fc0,
        0x2111090503fe0305, 0x4222120a07fd070a, 0x844424150efb0e15, 0x0888492a1cf71c2a, 0x1011925438ef3854, 0x212224a870df70a8, 0x42444850e0bfe050, 0x848890a0c07fc0a0,
        0x11090503fe030509, 0x22120a07fd070a12, 0x4424150efb0e1524, 0x88492a1cf71c2a49, 0x11925438ef385492, 0x2224a870df70a824, 0x444850e0bfe05048, 0x8890a0c07fc0a090,
        0x090503fe03050911, 0x120a07fd070a1222, 0x24150efb0e152444, 0x492a1cf71c2a4988, 0x925438ef38549211, 0x24a870df70a82422, 0x4850e0bfe0504844, 0x90a0c07fc0a09088,
        0x0503fe0305091121, 0x0a07fd070a122242, 0x150efb0e15244484, 0x2a1cf71c2a498808, 0x5438ef3854921110, 0xa870df70a8242221, 0x50e0bfe050484442, 0xa0c07fc0a0908884,
        0x03fe030509112141, 0x07fd070a12224282, 0x0efb0e1524448404, 0x1cf71c2a49880808, 0x38ef385492111010, 0x70df70a824222120, 0xe0bfe05048444241, 0xc07fc0a090888482,
        0xfe03050911214181, 0xfd070a1222428202, 0xfb0e152444840404, 0xf71c2a4988080808, 0xef38549211101010, 0xdf70a82422212020, 0xbfe0504844424140, 0x7fc0a09088848281
    };

    /* Omni-directional lookup attacks */
    static constexpr unsigned long long omnidir_attack_lookups [ 8 ] [ 64 ] =
    {
        {
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 0x0000000000000010, 0x0000000000000020, 0x0000000000000040,
            0x0000000000000000, 0x0000000000000100, 0x0000000000000201, 0x0000000000000402, 0x0000000000000804, 0x0000000000001008, 0x0000000000002010, 0x0000000000004020,
            0x0000000000000000, 0x0000000000010000, 0x0000000000020100, 0x0000000000040201, 0x0000000000080402, 0x0000000000100804, 0x0000000000201008, 0x0000000000402010,
            0x0000000000000000, 0x0000000001000000, 0x0000000002010000, 0x0000000004020100, 0x0000000008040201, 0x0000000010080402, 0x0000000020100804, 0x0000000040201008,
            0x0000000000000000, 0x0000000100000000, 0x0000000201000000, 0x0000000402010000, 0x0000000804020100, 0x0000001008040201, 0x0000002010080402, 0x0000004020100804,
            0x0000000000000000, 0x0000010000000000, 0x0000020100000000, 0x0000040201000000, 0x0000080402010000, 0x0000100804020100, 0x0000201008040201, 0x0000402010080402,
            0x0000000000000000, 0x0001000000000000, 0x0002010000000000, 0x0004020100000000, 0x0008040201000000, 0x0010080402010000, 0x0020100804020100, 0x0040201008040201
        },
        {
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080,
            0x0000000000000101, 0x0000000000000202, 0x0000000000000404, 0x0000000000000808, 0x0000000000001010, 0x0000000000002020, 0x0000000000004040, 0x0000000000008080,
            0x0000000000010101, 0x0000000000020202, 0x0000000000040404, 0x0000000000080808, 0x0000000000101010, 0x0000000000202020, 0x0000000000404040, 0x0000000000808080,
            0x0000000001010101, 0x0000000002020202, 0x0000000004040404, 0x0000000008080808, 0x0000000010101010, 0x0000000020202020, 0x0000000040404040, 0x0000000080808080,
            0x0000000101010101, 0x0000000202020202, 0x0000000404040404, 0x0000000808080808, 0x0000001010101010, 0x0000002020202020, 0x0000004040404040, 0x0000008080808080,
            0x0000010101010101, 0x0000020202020202, 0x0000040404040404, 0x0000080808080808, 0x0000101010101010, 0x0000202020202020, 0x0000404040404040, 0x0000808080808080,
            0x0001010101010101, 0x0002020202020202, 0x0004040404040404, 0x0008080808080808, 0x0010101010101010, 0x0020202020202020, 0x0040404040404040, 0x0080808080808080
        },
        {
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080, 0x0000000000000000,
            0x0000000000000204, 0x0000000000000408, 0x0000000000000810, 0x0000000000001020, 0x0000000000002040, 0x0000000000004080, 0x0000000000008000, 0x0000000000000000,
            0x0000000000020408, 0x0000000000040810, 0x0000000000081020, 0x0000000000102040, 0x0000000000204080, 0x0000000000408000, 0x0000000000800000, 0x0000000000000000,
            0x0000000002040810, 0x0000000004081020, 0x0000000008102040, 0x0000000010204080, 0x0000000020408000, 0x0000000040800000, 0x0000000080000000, 0x0000000000000000,
            0x0000000204081020, 0x0000000408102040, 0x0000000810204080, 0x0000001020408000, 0x0000002040800000, 0x0000004080000000, 0x0000008000000000, 0x0000000000000000,
            0x0000020408102040, 0x0000040810204080, 0x0000081020408000, 0x0000102040800000, 0x0000204080000000, 0x0000408000000000, 0x0000800000000000, 0x0000000000000000,
            0x0002040810204080, 0x0004081020408000, 0x0008102040800000, 0x0010204080000000, 0x0020408000000000, 0x0040800000000000, 0x0080000000000000, 0x0000000000000000
        },
        {
            0x0000000000000000, 0x0000000000000001, 0x0000000000000003, 0x0000000000000007, 0x000000000000000f, 0x000000000000001f, 0x000000000000003f, 0x000000000000007f,
            0x0000000000000000, 0x0000000000000100, 0x0000000000000300, 0x0000000000000700, 0x0000000000000f00, 0x0000000000001f00, 0x0000000000003f00, 0x0000000000007f00,
            0x0000000000000000, 0x0000000000010000, 0x0000000000030000, 0x0000000000070000, 0x00000000000f0000, 0x00000000001f0000, 0x00000000003f0000, 0x00000000007f0000,
            0x0000000000000000, 0x0000000001000000, 0x0000000003000000, 0x0000000007000000, 0x000000000f000000, 0x000000001f000000, 0x000000003f000000, 0x000000007f000000,
            0x0000000000000000, 0x0000000100000000, 0x0000000300000000, 0x0000000700000000, 0x0000000f00000000, 0x0000001f00000000, 0x0000003f00000000, 0x0000007f00000000,
            0x0000000000000000, 0x0000010000000000, 0x0000030000000000, 0x0000070000000000, 0x00000f0000000000, 0x00001f0000000000, 0x00003f0000000000, 0x00007f0000000000,
            0x0000000000000000, 0x0001000000000000, 0x0003000000000000, 0x0007000000000000, 0x000f000000000000, 0x001f000000000000, 0x003f000000000000, 0x007f000000000000,
            0x0000000000000000, 0x0100000000000000, 0x0300000000000000, 0x0700000000000000, 0x0f00000000000000, 0x1f00000000000000, 0x3f00000000000000, 0x7f00000000000000
        },
        {
            0x00000000000000fe, 0x00000000000000fc, 0x00000000000000f8, 0x00000000000000f0, 0x00000000000000e0, 0x00000000000000c0, 0x0000000000000080, 0x0000000000000000,
            0x000000000000fe00, 0x000000000000fc00, 0x000000000000f800, 0x000000000000f000, 0x000000000000e000, 0x000000000000c000, 0x0000000000008000, 0x0000000000000000,
            0x0000000000fe0000, 0x0000000000fc0000, 0x0000000000f80000, 0x0000000000f00000, 0x0000000000e00000, 0x0000000000c00000, 0x0000000000800000, 0x0000000000000000,
            0x00000000fe000000, 0x00000000fc000000, 0x00000000f8000000, 0x00000000f0000000, 0x00000000e0000000, 0x00000000c0000000, 0x0000000080000000, 0x0000000000000000,
            0x000000fe00000000, 0x000000fc00000000, 0x000000f800000000, 0x000000f000000000, 0x000000e000000000, 0x000000c000000000, 0x0000008000000000, 0x0000000000000000,
            0x0000fe0000000000, 0x0000fc0000000000, 0x0000f80000000000, 0x0000f00000000000, 0x0000e00000000000, 0x0000c00000000000, 0x0000800000000000, 0x0000000000000000,
            0x00fe000000000000, 0x00fc000000000000, 0x00f8000000000000, 0x00f0000000000000, 0x00e0000000000000, 0x00c0000000000000, 0x0080000000000000, 0x0000000000000000,
            0xfe00000000000000, 0xfc00000000000000, 0xf800000000000000, 0xf000000000000000, 0xe000000000000000, 0xc000000000000000, 0x8000000000000000, 0x0000000000000000
        },
        {
            0x0000000000000000, 0x0000000000000100, 0x0000000000010200, 0x0000000001020400, 0x0000000102040800, 0x0000010204081000, 0x0001020408102000, 0x0102040810204000,
            0x0000000000000000, 0x0000000000010000, 0x0000000001020000, 0x0000000102040000, 0x0000010204080000, 0x0001020408100000, 0x0102040810200000, 0x0204081020400000,
            0x0000000000000000, 0x0000000001000000, 0x0000000102000000, 0x0000010204000000, 0x0001020408000000, 0x0102040810000000, 0x0204081020000000, 0x0408102040000000,
            0x0000000000000000, 0x0000000100000000, 0x0000010200000000, 0x0001020400000000, 0x0102040800000000, 0x0204081000000000, 0x0408102000000000, 0x0810204000000000,
            0x0000000000000000, 0x0000010000000000, 0x0001020000000000, 0x0102040000000000, 0x0204080000000000, 0x0408100000000000, 0x0810200000000000, 0x1020400000000000,
            0x0000000000000000, 0x0001000000000000, 0x0102000000000000, 0x0204000000000000, 0x0408000000000000, 0x0810000000000000, 0x1020000000000000, 0x2040000000000000,
            0x0000000000000000, 0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
        },
        {
            0x0101010101010100, 0x0202020202020200, 0x0404040404040400, 0x0808080808080800, 0x1010101010101000, 0x2020202020202000, 0x4040404040404000, 0x8080808080808000,
            0x0101010101010000, 0x0202020202020000, 0x0404040404040000, 0x0808080808080000, 0x1010101010100000, 0x2020202020200000, 0x4040404040400000, 0x8080808080800000,
            0x0101010101000000, 0x0202020202000000, 0x0404040404000000, 0x0808080808000000, 0x1010101010000000, 0x2020202020000000, 0x4040404040000000, 0x8080808080000000,
            0x0101010100000000, 0x0202020200000000, 0x0404040400000000, 0x0808080800000000, 0x1010101000000000, 0x2020202000000000, 0x4040404000000000, 0x8080808000000000,
            0x0101010000000000, 0x0202020000000000, 0x0404040000000000, 0x0808080000000000, 0x1010100000000000, 0x2020200000000000, 0x4040400000000000, 0x8080800000000000,
            0x0101000000000000, 0x0202000000000000, 0x0404000000000000, 0x0808000000000000, 0x1010000000000000, 0x2020000000000000, 0x4040000000000000, 0x8080000000000000,
            0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
        },
        {
            0x8040201008040200, 0x0080402010080400, 0x0000804020100800, 0x0000008040201000, 0x0000000080402000, 0x0000000000804000, 0x0000000000008000, 0x0000000000000000,
            0x4020100804020000, 0x8040201008040000, 0x0080402010080000, 0x0000804020100000, 0x0000008040200000, 0x0000000080400000, 0x0000000000800000, 0x0000000000000000,
            0x2010080402000000, 0x4020100804000000, 0x8040201008000000, 0x0080402010000000, 0x0000804020000000, 0x0000008040000000, 0x0000000080000000, 0x0000000000000000,
            0x1008040200000000, 0x2010080400000000, 0x4020100800000000, 0x8040201000000000, 0x0080402000000000, 0x0000804000000000, 0x0000008000000000, 0x0000000000000000,
            0x0804020000000000, 0x1008040000000000, 0x2010080000000000, 0x4020100000000000, 0x8040200000000000, 0x0080400000000000, 0x0000800000000000, 0x0000000000000000,
            0x0402000000000000, 0x0804000000000000, 0x1008000000000000, 0x2010000000000000, 0x4020000000000000, 0x8040000000000000, 0x0080000000000000, 0x0000000000000000,
            0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
        }
    };



    /* INTERNAL METHODS */

    /** @name  shift_val, shift_mask
     * 
     * @brief  Get a shift value or mask from a compass direction
     * @param  dir: Compass direction
     * @return Shift value or mask
     */
    static constexpr int shift_val ( compass dir )        noexcept { return shift_vals        [ cast_compass ( dir ) ]; }
    static constexpr int shift_val ( knight_compass dir ) noexcept { return knight_shift_vals [ cast_compass ( dir ) ]; }
    static constexpr bitboard shift_mask ( compass dir )        noexcept { return bitboard { shift_masks        [ cast_compass ( dir ) ] }; }
    static constexpr bitboard shift_mask ( knight_compass dir ) noexcept { return bitboard { knight_shift_masks [ cast_compass ( dir ) ] }; }

    /** @name  singleton_bitset
     * 
     * @brief  Create a 64-bit integer with one bit set
     * @param  pos:  The absolute position [0,63]
     * @param  rank: The rank of the bit [0,7]
     * @param  file: The file of the bit [0,7]
     * @return 64-bit integer
     */
    static constexpr unsigned long long singleton_bitset ( unsigned pos ) noexcept { return 1ull << pos; }
    static constexpr unsigned long long singleton_bitset ( unsigned rank, unsigned file ) noexcept { return 1ull << ( rank * 8 + file ); }

};



/* INCLUDE INLINE IMPLEMENTATION */
#include <chess/bitboard.hpp>



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_H_INCLUDED */