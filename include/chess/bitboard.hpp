/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * include/chess/bitboard.hpp
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



/** @name  compass_start, knight/straight/diagonal_compass_start
 *
 * @brief  Gives the first direction in a compass (useful for iterating over a compass)
 * @return A compass
 */
inline constexpr chess::compass chess::compass_start () noexcept { return compass::sw; }
inline constexpr chess::knight_compass chess::knight_compass_start () noexcept { return knight_compass::ssw; }
inline constexpr chess::straight_compass chess::straight_compass_start () noexcept { return straight_compass::s; }
inline constexpr chess::diagonal_compass chess::diagonal_compass_start () noexcept { return diagonal_compass::sw; }

/** @name  compass_next
 *
 * @brief  Gives the next direction in a compass, looping back to the start if reaching the end (useful for iterating over a compass)
 * @param  dir: The compass direction to advance.
 * @return A compass
 */
inline constexpr chess::compass chess::compass_next ( compass dir ) noexcept { return static_cast<compass> ( ( static_cast<int> ( dir ) + 1 ) & 7 ); }
inline constexpr chess::knight_compass chess::compass_next ( knight_compass dir ) noexcept { return static_cast<knight_compass> ( ( static_cast<int> ( dir ) + 1 ) & 7 ); }
inline constexpr chess::straight_compass chess::compass_next ( straight_compass dir ) noexcept 
{ 
    bool add_one = ( static_cast<int> ( dir ) == 3 );
    return static_cast<straight_compass> ( ( static_cast<int> ( dir ) + 2 - add_one ) & 7 );
}
inline constexpr chess::diagonal_compass chess::compass_next ( diagonal_compass dir ) noexcept 
{ 
    bool add_one = ( static_cast<int> ( dir ) == 7 );
    bool add_three = ( static_cast<int> ( dir ) == 2 );
    return static_cast<diagonal_compass> ( ( static_cast<int> ( dir ) + 2 - add_one + add_three ) & 7 );
}



/* OTHER BITWISE OPERATIONS */



/** @name  popcount
 * 
 * @return integer
 */
inline constexpr unsigned chess::bitboard::popcount () const noexcept 
{
    /* STL function adds a branch testing for x = 0, which is likely to be undesirable */
#ifdef CHESS_BUILTIN_POPCOUNT64 
    return CHESS_BUILTIN_POPCOUNT64 ( bits ); 
#else
    return std::popcount ( bits );
#endif
}

/** @name  hamming_dist
 * 
 * @param  other: Another bitboard
 * @return The number of different bits comparing this and other
 */
inline constexpr unsigned chess::bitboard::hamming_dist ( bitboard other ) const noexcept 
{ 
    /* STL function adds a branch testing for x = 0, which is likely to be undesirable */
#ifdef CHESS_BUILTIN_POPCOUNT64 
    return CHESS_BUILTIN_POPCOUNT64 ( bits ^ other.bits ); 
#else
    return std::popcount ( bits ^ other.bits );
#endif
}

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
    constexpr bitboard k1 { 0x00ff00ff00ff00ff }, k2 { 0x0000ffff0000ffff };
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
    constexpr bitboard k1 { 0x5555555555555555 }, k2 { 0x3333333333333333 }, k4 { 0x0f0f0f0f0f0f0f0f };
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
    constexpr bitboard k1 { 0x5500550055005500 }, k2 { 0x3333000033330000 }, k4 { 0x0f0f0f0f00000000 };
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
    constexpr bitboard k1 { 0xaa00aa00aa00aa00 }, k2 { 0xcccc0000cccc0000 }, k4 { 0xf0f0f0f00f0f0f0f };
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
    constexpr bitboard k1 { 0xaaaaaaaaaaaaaaaa }, k2 { 0xcccccccccccccccc }, k4 { 0xf0f0f0f0f0f0f0f0 };
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
    constexpr bitboard k1 { 0x5555555555555555 }, k2 { 0x3333333333333333 }, k4 { 0x0f0f0f0f0f0f0f0f };
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
    /* x will store the output, prev will remember the result of the previous iteration */
    bitboard x { bits }, prev; 
    do {
        /* Starting new iteration, so copy x over to prev.
         * Union the east and west shift of x with itself.
         * Union the north and south shift of x with itself, so that x has expanded outward by 1 cell in each direction.
         * Check with the propagator and repeat until no change has been made.
         */
        prev = x;
        x |= x.shift ( compass::w ) | x.shift ( compass::e );
        x |= x.shift ( compass::s ) | x.shift ( compass::n );
        x &= p;
    } while ( x != prev );

    /* return the flood */
    return x;
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
    /* x will store the output, prev will remember the result of the previous iteration */
    bitboard x { bits }, prev;
    do {
        /* Starting new iteration, so copy x over to prev.
         * Union the east and west shift of x with itself.
         * Union the north and south shift of x with itself, so that x has expanded outward by 1 cell in each direction.
         * Check with the propagator.
         * If all targets are now found within the flood then return true, else repeat until no change has been made.
         */
        prev = x;                                            
        x |= x.shift ( compass::w ) | x.shift ( compass::e );
        x |= x.shift ( compass::s ) | x.shift ( compass::n );
        x &= p;                              
        if ( x.contains ( t ) ) return true;
    } while ( x != prev );

    /* not all targets found so return false */           
    return false;                            
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
    /* A legal double south push will leave a pawn in rank 5.
     * For the first push, shift the board north one, check with propagator and store in x.
     * For the second push, shift the board north one again, check with propagator and k1, then return the union with x.
     */
    constexpr bitboard k1 { masks::rank_4 };            
    bitboard x { shift ( compass::n ) & p };            
    return { x | ( x.shift ( compass::n ) & p & k1 ) }; 
}
inline constexpr chess::bitboard chess::bitboard::pawn_push_s ( bitboard p ) const noexcept
{
    /* A legal double south push will leave a pawn in rank 5.
     * For the first push, shift the board south one, check with propagator and store in x.
     * For the second push, shift the board south one again, check with propagator and k1, then return the union with x.
     */
    constexpr bitboard k1 { masks::rank_5 };           
    bitboard x { shift ( compass::s ) & p };           
    return { x | ( x.shift ( compass::s ) & p & k1 ) };
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
    /* If was declared as a singleton set, simply look up and return the attacks of the king */
    if ( single ) return king_attack_lookup ( trailing_zeros_nocheck () ); else
    {
        /* x will store the output, t will help form the output.
         * Set the east and west shifts to x, then union them back to t.
         * Since t now comprises of the initial, east and west shifts, the shifting north and south gives the remaining 6 attack cells.
         * Check with the propagator and return x.
         */
        bitboard x, t { bits };                               
        x  = t.shift ( compass::w ) | t.shift ( compass::e ); 
        t |= x;                                               
        x |= t.shift ( compass::s ) | t.shift ( compass::n ); 
        return x & p;                                         
    }
}



/* ROOK BISHOP AND QUEEN MOVES */



/** @name  rook/bishop/queen_any_attack
 * 
 * @brief  Gives the intersection of attacks in all directions
 * @param  pp: Primary propagator set: set bits are where the board is allowed to flow without capture, universe by default
 * @param  sp: Secondary propagator set: set bits are where the board is allowed to flow or capture, empty by default.
 *             Should technically be a superset of pp, however ( pp | sp ) is used rather than sp alone, sp can simply be the set of capturable pieces.
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::rook_all_attack   ( bitboard pp, bitboard sp ) const noexcept
{
    /* x will store the output, dir iterates over the directions of the compass */
    bitboard x;
    straight_compass dir = straight_compass_start ();

    /* Iterate over the 4 compass directions, forcing GCC to unroll the loop.
     * This causes dir to be recognised as a constant expression which will greatly optimise the loop.
     */
    #pragma GCC unroll 4
    for ( int i = 0; i < 4; ++i )
    {
        /* Union the attacks in this direction to x, and increment dir */
        x |= rook_attack ( dir, pp, sp );
        dir = compass_next ( dir );
    }

    /* Return the union of all attacks */
    return x;
}
inline constexpr chess::bitboard chess::bitboard::bishop_all_attack ( bitboard pp, bitboard sp ) const noexcept
{
    /* x will store the output, dir iterates over the directions of the compass */
    bitboard x;
    diagonal_compass dir = diagonal_compass_start ();

    /* Iterate over the 4 compass directions, forcing GCC to unroll the loop.
     * This causes dir to be recognised as a constant expression which will greatly optimise the loop.
     */
    #pragma GCC unroll 4
    for ( int i = 0; i < 4; ++i )
    {
        /* Union the attacks in this direction to x, and increment dir */
        x |= bishop_attack ( dir, pp, sp );
        dir = compass_next ( dir );
    }

    /* Return the union of all attacks */
    return x;
}
inline constexpr chess::bitboard chess::bitboard::queen_all_attack  ( bitboard pp, bitboard sp ) const noexcept
{
    /* x will store the output, dir iterates over the directions of the compass */
    bitboard x;
    compass dir = compass_start ();

    /* Iterate over the 8 compass directions, forcing GCC to unroll the loop.
     * This causes dir to be recognised as a constant expression which will greatly optimise the loop.
     */
    #pragma GCC unroll 8
    for ( int i = 0; i < 8; ++i )
    {
        /* Union the attacks in this direction to x, and increment dir */
        x |= queen_attack ( dir, pp, sp );
        dir = compass_next ( dir );
    }

    /* Return the union of all attacks */
    return x;
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
    /* x will store the output, temp allows for the iteration through the set bits of this */
    bitboard x, temp { bits }; 

    /* While there are set bits left in temp (i.e. knights on the board), continue to add to x */
    while ( temp )
    {
        /* The position of the next set bit in temp is given by the number of trailing zeros.
         * Lookup the knight attacks at pos and union them with x, then reset the bit in temp.
         */
        int pos = temp.trailing_zeros_nocheck (); 
        x |= knight_attack_lookup ( pos );        
        temp.reset ( pos );                       
    }

    /* Check with the propagator and return x */
    return x & p;
}

/** @name  knight_mult_attack
 *
 * @brief  Gives the set of cells attacked by more than one knight
 * @param  p: Propagator set: set bits are empty cells or capturable pieces, universe by default
 * @return A new bitboard
 */
inline constexpr chess::bitboard chess::bitboard::knight_mult_attack ( bitboard p ) const noexcept
{
    /* once and mult will remember if a cell is attacked once or multiple times, temp allows for the iteration through the set bits of this */
    bitboard once, mult, temp { bits };

    /* While there are set bits left in temp (i.e. kings on the board), continue to add to x */
    while ( temp )
    {
        /* The position of the next set bit in temp is given by the number if trailing zeros.
         * Lookup the knight attacks at pos, and union them with mult only if any of the cells have alreay been attacked once.
         * Union the attacked cells with once then reset the bit in temp.
         */
        int pos = temp.trailing_zeros_nocheck ();   
        mult |= knight_attack_lookup ( pos ) & once;
        once |= knight_attack_lookup ( pos );       
        temp.reset ( pos );                         
    }

    /* Check with the propagator and return x */
    return mult & p;
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_BITBOARD_HPP_INCLUDED */