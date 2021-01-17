/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * src/chess/bitboard.hpp
 *
 * Inline implementation of include/chess/bitboard.h
 *
 */



/* HEADER GUARD */
#ifndef CHESS_BITBOARD_HPP_INCLUDED
#define CHESS_BITBOARD_HPP_INCLUDED



/* INCLUDES */
#include <chess/bitboard.h>



/* COMPASSES */



/** @name  compass_start, knight_compass_start
 *
 * @brief  Gives the first direction in a compass (useful for iterating over a compass)
 * @return A compass
 */
inline constexpr chess::compass chess::compass_start () noexcept { return static_cast<compass> ( 0 ); }
inline constexpr chess::knight_compass chess::knight_compass_start () noexcept { return static_cast<knight_compass> ( 0 ); }

/** @name  compass_next
 *
 * @brief  Gives the next direction in a compass, looping back to the start if reaching the end (useful for iterating over a compass)
 * @param  dir: The compass direction to advance.
 * @return A compass
 */
inline constexpr chess::compass chess::compass_next ( compass dir ) noexcept { return static_cast<compass> ( ( static_cast<int> ( dir ) + 1 ) & 7 ); }
inline constexpr chess::knight_compass chess::compass_next ( knight_compass dir ) noexcept { return static_cast<knight_compass> ( ( static_cast<int> ( dir ) + 1 ) & 7 ); }



/* OTHER BITWISE OPERATIONS */



/** @name  leading/trailing_zeros_nocheck
 * 
 * @return The number of leading/trailing zeros, but undefined if the bitboard is empty
 */
inline constexpr unsigned chess::bitboard::leading_zeros_nocheck  () const noexcept
{
    /* Use builtin if availible, otherwise use the checking method */
#ifdef CHESS_BUILTIN_CLZ64
    return CHESS_BUILTIN_CLZ64 ( bits );
#else
    return leading_zeros ();
#endif
}
inline constexpr unsigned chess::bitboard::trailing_zeros_nocheck () const noexcept
{
    /* Use builtin if availible, otherwise use the checking method */
#ifdef CHESS_BUILTIN_CTZ64
    return CHESS_BUILTIN_CTZ64 ( bits );
#else
    return trailing_zeros ();
#endif
}

/** @name  vertical_flip
 *
 * @brief  Flip the board vertically
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::vertical_flip () const noexcept
{
    /* Use builtin if availible, otherwise compute manually */
#ifdef CHESS_BUILTIN_BSWAP64
    return bitboard { CHESS_BUILTIN_BSWAP64 ( bits ) };
#else
    constexpr bitboard k1 { 0x00FF00FF00FF00FF }, k2 { 0x0000FFFF0000FFFF };
    bitboard x { bits };
    x = ( ( x >>  8 ) & k1 ) | ( ( x & k1 ) <<  8 );
    x = ( ( x >> 16 ) & k2 ) | ( ( x & k2 ) << 16 );
    x = ( ( x >> 32 )      ) | ( ( x      ) << 32 );
    return x;
#endif
}

/** @name  horizontal_flip
 *
 * @brief  Flip the board horizontally
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::horizontal_flip () const noexcept
{
    constexpr bitboard k1 { 0x5555555555555555 }, k2 { 0x3333333333333333 }, k4 { 0x0F0F0F0F0F0F0F0F };
    bitboard x { bits };
    x ^= k4 & ( x ^ x.bit_rotl ( 8 ) );
    x ^= k2 & ( x ^ x.bit_rotl ( 4 ) );
    x ^= k1 & ( x ^ x.bit_rotl ( 2 ) );
    return x.bit_rotr ( 7 );
}

/** @name  pos_diag_flip
 *
 * @brief  Flip the board along y=x
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pos_diag_flip () const noexcept
{
    constexpr bitboard k1 { 0x5500550055005500 }, k2 { 0x3333000033330000 }, k4 { 0x0F0F0F0F00000000 };
    bitboard x { bits }, t;
    t  = k4 & ( x ^ ( x << 28 ) );
    x ^=      ( t ^ ( t >> 28 ) );
    t  = k2 & ( x ^ ( x << 14 ) );
    x ^=      ( t ^ ( t >> 14 ) );
    t  = k1 & ( x ^ ( x <<  7 ) );
    x ^=      ( t ^ ( t >>  7 ) );
    return x;
}

/** @name  neg_diag_flip
 *
 * @brief  Flip the board along y=-x
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Flip_and_Mirror
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::neg_diag_flip () const noexcept
{
    constexpr bitboard k1 { 0xAA00AA00AA00AA00 }, k2 { 0xCCCC0000CCCC0000 }, k4 { 0xF0F0F0F00F0F0F0F };
    bitboard x { bits }, t;
    t  =      ( x ^ ( x << 36 ) );
    x ^= k4 & ( t ^ ( x >> 36 ) );
    t  = k2 & ( x ^ ( x << 18 ) );
    x ^=      ( t ^ ( t >> 18 ) );
    t  = k1 & ( x ^ ( x <<  9 ) );
    x ^=      ( t ^ ( t >>  9 ) );
    return x;
}

/** @name  pseudo_rotate_45_clock
 *
 * @brief  Flip the positive diagonals to ranks
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pseudo_rotate_45_clock () const noexcept
{
    constexpr bitboard k1 { 0xAAAAAAAAAAAAAAAA }, k2 { 0xCCCCCCCCCCCCCCCC }, k4 { 0xF0F0F0F0F0F0F0F0 };
    bitboard x { bits };
    x ^= k1 & ( x ^ x.bit_rotr (  8 ) );
    x ^= k2 & ( x ^ x.bit_rotr ( 16 ) );
    x ^= k4 & ( x ^ x.bit_rotr ( 32 ) );
    return x;
}

/** @name  pseudo_rotate_45_aclock
 *
 * @brief  Flip the negative diagonals to ranks
 * @see    https://www.chesspgramming.org/Flipping_Mirroring_and_Rotating#Pseudo-Rotation_by_45_degrees
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pseudo_rotate_45_aclock () const noexcept
{
    constexpr bitboard k1 { 0x5555555555555555 }, k2 { 0x3333333333333333 }, k4 { 0x0F0F0F0F0F0F0F0F };
    bitboard x { bits };
    x ^= k1 & ( x ^ x.bit_rotr (  8 ) );
    x ^= k2 & ( x ^ x.bit_rotr ( 16 ) );
    x ^= k4 & ( x ^ x.bit_rotr ( 32 ) );
    return x;
}



/* GENERIC FILL ALGORITHMS */



/** @name  fill
 *
 * @brief  Fill the board in a given direction taking into account occluders
 * @see    https://www.chessprogramming.org/Kogge-Stone_Algorithm#OccludedFill
 * @param  dir: The direction to shift
 * @param  p: Propagator set: set bits are where the board is allowed to flow, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::fill ( compass dir, bitboard p ) const noexcept
{
    bitboard x { bits }; int r = shift_val ( dir );
    p &=     shift_mask ( dir );
    x |= p & x.bitshift ( r );
    p &=     p.bitshift ( r );
    x |= p & x.bitshift ( r * 2 );
    p &=     p.bitshift ( r * 2 );
    x |= p & x.bitshift ( r * 4 );
    return x;
}

/** @name  flood_fill
 *
 * @brief  Fill the board in all directions until all positions reachable from the current are found.
 * @see    https://www.chessprogramming.org/King_Pattern#Flood_Fill_Algorithms
 * @param  p: Propagator set: set bits are where the board is allowed to flow.
 *         Note: a piece can move over a diagonal boundary (like it would with a diagonal fill).
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::flood_fill ( bitboard p ) const noexcept
{
    bitboard x { bits }, prev; // x will store the output, prev will remember the result of the previous iteration.
    do {
        prev = x;                                             // Starting new iteration, so copy x over to prev.
        x |= x.shift ( compass::w ) | x.shift ( compass::e ); // Union the east and west shift of x with itself.
        x |= x.shift ( compass::s ) | x.shift ( compass::n ); // Union the north and south shift of x with itself, so that x has expanded outward by 1 cell in each direction.
        x &= p;            // Check with propagator.
    } while ( x != prev ); // Repeat until no change has been made.
    return x;              // Return x.
}

/** @name  is_connected
 *
 * @brief  Use a flood fill algorithm to test whether a set of targets can all be reached
 * @see    https://www.chessprogramming.org/King_Pattern#Flood_Fill_Algorithms
 * @param  p: Propagator set: set bits are where the board is allowed to flow.
 *         Note: a piece can move over a diagonal boundary (like it would with a diagonal fill).
 * @param  t: Target set: if all set bits can be reached then true is returned, false otherwise
 * @return boolean
 */
inline constexpr bool chess::bitboard::is_connected ( bitboard p, bitboard t ) const noexcept
{
    bitboard x { bits }, prev; // x will store the output, prev will remember the result of the previous iteration.
    do {
        prev = x;                                             // Starting new iteration, so copy x over to prev.
        x |= x.shift ( compass::w ) | x.shift ( compass::e ); // Union the east and west shift of x with itself.
        x |= x.shift ( compass::s ) | x.shift ( compass::n ); // Union the north and south shift of x with itself, so that x has expanded outward by 1 cell in each direction.
        x &= p;                              // Check with propagator.
        if ( x.contains ( t ) ) return true; // If all targets are now found within the flood, return true.
    } while ( x != prev );                   // Repeat until no change has been made.
    return false;                            // Not all targets reached, so return false.
}



/* PAWN MOVES */



/** @name  pawn_push_n/s
 *
 * @brief  Gives the span of pawn pushes, including double pushes
 * @see    https://www.chessprogramming.org/Pawn_Pushes_(Bitboards)#Push_per_Side
 * @param  p: Propagator set: set bits are empty cells, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::pawn_push_n ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::rank_4 };            // A legal double south push will leave a pawn in rank 5.
    bitboard x { shift ( compass::n ) & p };            // For the first push, shift the board north one, check with propagator and store in x.
    return { x | ( x.shift ( compass::n ) & p & k1 ) }; // For the second push, shift the board north one again, check with propagator and k1, then return the union with x.
}
inline constexpr chess::bitboard chess::bitboard::pawn_push_s ( bitboard p ) const noexcept
{
    constexpr bitboard k1 { masks::rank_5 };            // A legal double south push will leave a pawn in rank 5.
    bitboard x { shift ( compass::s ) & p };            // For the first push, shift the board south one, check with propagator and store in x.
    return { x | ( x.shift ( compass::s ) & p & k1 ) }; // For the second push, shift the board south one again, check with propagator and k1, then return the union with x.
}



/* KNIGHT MOVES */



/** @name  knight_any_attack
 *
 * @brief  Gives the union of all knight attacks
 * @see    https://www.chessprogramming.org/Knight_Pattern#Multiple_Knight_Attacks
 * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::knight_any_attack ( bitboard p ) const noexcept
{
    bitboard x, temp { bits }; // x will store the output, temp allows for the iteration through the set bits of this.
    while ( temp )             // While there are set bits left in temp, continue to add to x.
    {
        int pos = temp.trailing_zeros_nocheck (); // The position of the next set bit in temp is given by the number if trailing zeros.
        x |= knight_attack_lookup ( pos );        // Lookup the knight attacks at pos, and union them with x.
        temp.reset ( pos );                       // Reset the bit in temp.
    }
    return x & p; // Check with the propagator and return x.
}

/** @name  knight_mult_attack
 *
 * @brief  Gives the set of cells attacked by more than one knight
 * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::knight_mult_attack ( bitboard p ) const noexcept
{
    bitboard once, mult, temp { bits }; // once and mult will remember if a cell is attacked once or multiple times, temp allows for the iteration through the set bits of this.
    while ( temp )                      // While there are set bits left in temp, continue to add to x.
    {
        int pos = temp.trailing_zeros_nocheck ();    // The position of the next set bit in temp is given by the number if trailing zeros.
        mult |= knight_attack_lookup ( pos ) & once; // Lookup the knight attacks at pos, and union them with mult only if any of the cells have alreay been attacked once.
        once |= knight_attack_lookup ( pos );        // Union the attacked cells with once.
        temp.reset ( pos );                          // Reset the bit in temp.
    }
    return mult & p; // Check with the propagator and return x.
}



/* KING MOVES */



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
inline constexpr chess::bitboard chess::bitboard::king_any_attack ( bitboard p, bool single ) const noexcept
{
    if ( single ) return king_attack_lookup ( trailing_zeros_nocheck () ); else // If was declared as a singleton set, simply look up and return the attacks of the king.
    {
        bitboard x, t { bits };                                // x will store the output, t will help form the output.
        x  = t.shift ( compass::w ) | t.shift ( compass::e );  // Set the east and west shifts to x.
        t |= x;                                                // Union them back to t.
        x |= t.shift ( compass::s ) | t.shift ( compass::n );  // Since t now comprises of the initial, east and west shifts, the shifting north and south gives the remaining 6 attack cells.
        return x & p;                                          // Check with the propagator and return x.
    }
}




/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_HPP_INCLUDED */