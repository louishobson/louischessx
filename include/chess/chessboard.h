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
     * store and manipulate a bitboard-based chess board
     */
    class chessboard;
}



/* CHESSBOARD DEFINITION */

/* class chessboard 
 *
 * store and manipulate a bitboard-based chess board
 */
class chess::chessboard
{
public:

    /* TYPES */

    /* enum piece_type
     *
     * enum values for the different bitboards
     */
    enum class piece_type : int
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
     * @brief  sets up an opening chess board
     */
    constexpr chessboard () : boards
    {
        0x000000000000FFFF,
        0xFFFF000000000000,
        0x00FF00000000FF00,
        0x2400000000000024,
        0x4200000000000042,
        0x8100000000000081,
        0x1000000000000008,
        0x0800000000000010
    } {}



    /* BITBOARD ACCESS */

    /** @name  get
     * 
     * @brief  gets a bitboard based on a single piece type
     * @param  pt: one of piece_type
     * @return the bitboard for pt 
     */
    bitboard& get ( const piece_type pt ) { return boards.at ( ptoi ( pt ) ); } 
    const bitboard& get ( const piece_type pt ) const { return boards.at ( ptoi ( pt ) ); }

    /** @name  get_comp
     * 
     * @brief  gets a composite bitboard from the intersection of two bitboards
     * @param  pt0: one of piece_type 
     * @param  pt1: one of piece_type
     * @return the bitboard for pt 
     */
    bitboard get ( const piece_type pt0, const piece_type pt1 ) const { return boards.at ( ptoi ( pt0 ) ) & boards.at ( ptoi ( pt0 ) ); }



private:

    /* ATTRIBUTES */

    /* array of bitboards for the board */
    std::array<bitboard, 8> boards;



    /* INTERNAL METHODS */

    /** @name  ptoi
     * 
     * @brief  converts a piece_type enum to an int
     * @param  pt: the piece_type to convert
     * @return int
     */
    constexpr int ptoi ( const piece_type pt ) const noexcept { return static_cast<int> ( pt ); }

};



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */