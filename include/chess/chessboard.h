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
 * https://www.chessprogramming.org/Pawn_Attacks_(Bitboards)#Attack_Maps
 * https://www.chessprogramming.org/Pawn_Spans#Interspans
 * https://www.chessprogramming.org/Pawn_Rams_(Bitboards)
 * https://www.chessprogramming.org/Pawn_Levers_(Bitboards)
 * 
 */



/* HEADER GUARD */
#ifndef CHESSBOARD_H_INCLUDED
#define CHESSBOARD_H_INCLUDED



/* INCLUDES */
#include <array>
#include <cctype>
#include <chess/bitboard.h>
#include <string>



/* DECLARATIONS */

namespace chess
{
    /* enum bbtype
     *
     * Enum values for the different bitboards
     */
    enum class bbtype
    {
        white,
        black,
        pawn,
        king,
        queen,
        bishop,
        knight,
        rook,
        no_piece
    };

    /* enum pcolor
     *
     * Enum values for different colors of piece (not types)
     */
    enum class pcolor
    {
        white,
        black,
        no_piece = 8,
    };

    /* enum ptype
     *
     * Enum values for different types of piece (not colors)
     */
    enum class ptype
    {
        pawn = 2,
        king,
        queen,
        bishop,
        knight,
        rook,
        no_piece
    };

    /* class chessboard 
     *
     * Store and manipulate a bitboard-based chess board
     */
    class chessboard;
}



/* CHESSBOARD DEFINITION */

/* class chessboard 
 *
 * Store and manipulate a bitboard-based chess board
 */
class chess::chessboard
{
public:

    /* CONSTRUCTORS */

    /** @name  default constructor
     * 
     * @brief  Sets up an opening chess board
     */
    constexpr chessboard () : bbs
    {
        bitboard { 0x000000000000ffff },
        bitboard { 0xffff000000000000 },
        bitboard { 0x00ff00000000ff00 },
        bitboard { 0x0800000000000010 },
        bitboard { 0x1000000000000008 },
        bitboard { 0x2400000000000024 },
        bitboard { 0x4200000000000042 },
        bitboard { 0x8100000000000081 }
    } {}



    /* FORMATTING */

    /** @name  simple_format_board
     * 
     * @brief  Create a simple representation of the board.
     *         Lower-case letters mean black, upper case white.
     * @return string
     */
    std::string simple_format_board () const;



//private:

    /* ATTRIBUTES */

    /* Array of bitboards for the board */
    std::array<bitboard, 8> bbs;



    /* BITBOARD ACCESS */

    /** @name  get_bb
     * 
     * @brief  Gets a bitboard, by reference, based on a single piece type
     * @param  pt: One of bbtype, pcolor or ptype. Undefined behaviour if is no_piece.
     * @return The bitboard for pt 
     */
    bitboard& get_bb ( bbtype pt ) noexcept { return bbs [ static_cast<int> ( pt ) ]; } 
    bitboard& get_bb (  ptype pt ) noexcept { return bbs [ static_cast<int> ( pt ) ]; } 
    bitboard& get_bb ( pcolor pt ) noexcept { return bbs [ static_cast<int> ( pt ) ]; } 
    const bitboard& get_bb ( bbtype pt ) const noexcept { return bbs [ static_cast<int> ( pt ) ]; }
    const bitboard& get_bb (  ptype pt ) const noexcept { return bbs [ static_cast<int> ( pt ) ]; }
    const bitboard& get_bb ( pcolor pt ) const noexcept { return bbs [ static_cast<int> ( pt ) ]; }

    /** @name  bb
     * 
     * @brief  Gets a bitboard, by copy, based on a single piece type
     * @param  pt: One of bbtype, pcolor or ptype. Undefined behaviour if is no_piece.
     * @return The bitboard for pt
     */
    bitboard bb ( bbtype pt ) const noexcept { return bbs [ static_cast<int> ( pt ) ]; }
    bitboard bb (  ptype pt ) const noexcept { return bbs [ static_cast<int> ( pt ) ]; }
    bitboard bb ( pcolor pt ) const noexcept { return bbs [ static_cast<int> ( pt ) ]; }

    /** @name  white_bb, black_bb 
     * 
     * @brief  Gets a bitboard from the intersection of a colour and another bitboard
     * @param  pt: One of ptype. Undefined behaviour if is no_piece.
     * @return A new bitboard
     */
    bitboard white_bb ( ptype pt ) const noexcept { return bbs [ static_cast<int> ( bbtype::white ) ] & bbs [ static_cast<int> ( pt ) ]; }
    bitboard black_bb ( ptype pt ) const noexcept { return bbs [ static_cast<int> ( bbtype::black ) ] & bbs [ static_cast<int> ( pt ) ]; }

    /** @name  occupied_bb
     * 
     * @brief  Gets the union of white and black pieces
     * @return A new bitboard
     */
    bitboard occupied_bb () const noexcept { return bb ( bbtype::white ) | bb ( bbtype::black ); }



    /* FURTHER BITBOARD MANIPULATION */

    /** @name  pawn_interspan_bb
     * 
     * @brief  Get the interspan of the pawns
     * @return A new bitboard
     */
    [[ gnu::flatten ]]
    bitboard pawn_interspan_bb () const noexcept { return white_bb ( ptype::pawn ).span ( compass::n ) & black_bb ( ptype::pawn ).span ( compass::s ); }



    /* BOARD LOOKUP */

    /** @name  find_color
     *
     * @brief  Determines the color of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pos: Board position
     * @return One of pcolor
     */
    pcolor find_color ( unsigned pos ) const noexcept;

    /** @name  find_type
     * 
     * @brief  Determines the type of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pos: Board position
     * @return One of ptype
     */
    ptype find_type ( unsigned pos ) const noexcept;

};



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */