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



    /* BITBOARD ACCESS */

    /** @name  get
     * 
     * @brief  Gets a bitboard based on a single piece type
     * @param  pt: One of piece_type
     * @return The bitboard for pt 
     */
    bitboard& get ( piece_type pt ) { return boards.at ( ptoi ( pt ) ); } 
    const bitboard& get ( piece_type pt ) const { return boards.at ( ptoi ( pt ) ); }

    /** @name  get_comp
     * 
     * @brief  Gets a composite bitboard from the intersection of two bitboards
     * @param  pt0: One of piece_type 
     * @param  pt1: One of piece_type
     * @return The bitboard for pt 
     */
    bitboard get ( piece_type pt0, const piece_type pt1 ) const { return boards.at ( ptoi ( pt0 ) ) & boards.at ( ptoi ( pt0 ) ); }



private:

    /* ATTRIBUTES */

    /* Array of bitboards for the board */
    std::array<bitboard, 8> boards;



    /* INTERNAL METHODS */

    /** @name  ptoi
     * 
     * @brief  Converts a piece_type enum to an int
     * @param  pt: The piece_type to convert
     * @return integer
     */
    constexpr int ptoi ( piece_type pt ) const noexcept { return static_cast<int> ( pt ); }

};



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */