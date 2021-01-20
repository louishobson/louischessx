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
#include <chess/bitboard.h>



/* DECLARATIONS */

namespace chess
{
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

    /* TYPES */

    /* enum piece_type
     *
     * Enum values for the different bitboards
     */
    enum class piece_type
    {
        white,
        black,
        pawn,
        knight,
        bishop,
        rook,
        queen,
        king
    };



    /* CONSTRUCTORS */

    /** @name  default constructor
     * 
     * @brief  Sets up an opening chess board
     */
    constexpr chessboard () : boards
    {
        bitboard { 0x000000000000FFFF },
        bitboard { 0xFFFF000000000000 },
        bitboard { 0x00FF00000000FF00 },
        bitboard { 0x2400000000000024 },
        bitboard { 0x4200000000000042 },
        bitboard { 0x8100000000000081 },
        bitboard { 0x1000000000000008 },
        bitboard { 0x0800000000000010 }
    } {}



private:

    /* ATTRIBUTES */

    /* Array of bitboards for the board */
    std::array<bitboard, 8> boards;



    /* BITBOARD ACCESS */

    /** @name  get_bb
     * 
     * @brief  Gets a bitboard based on a single piece type
     * @param  pt: One of piece_type
     * @return The bitboard for pt 
     */
    bitboard& get_bb ( piece_type pt ) noexcept { return boards [ static_cast<int> ( pt ) ]; } 
    const bitboard& get_bb ( piece_type pt ) const noexcept { return boards [ static_cast<int> ( pt ) ]; }

    /** @name  get_bb 
     * 
     * @brief  Gets a composite bitboard from the intersection of two bitboards
     * @param  pt0: One of piece_type 
     * @param  pt1: One of piece_type
     * @return The bitboard for pt 
     */
    bitboard get_bb ( piece_type pt0, piece_type pt1 ) const noexcept { return boards [ static_cast<int> ( pt0 ) ] & boards [ static_cast<int> ( pt0 ) ]; }

};



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */