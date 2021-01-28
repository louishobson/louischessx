/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/chessboard.cpp
 * 
 * Implementation of include/chess/chessboard.h
 * 
 */



/* INCLUDES */
#include <chess/chessboard.h>



/* FORMATTING */

/** @name  simple_format_board
 * 
 * @brief  Create a simple representation of the board.
 *         Lower-case letters mean black, upper case white.
 * @return string
 */
std::string chess::chessboard::simple_format_board () const
{
    /* Add to string one bit at a time.
     * i ^ 56 changes the endianness of i, such that the top row is read first.
     * Multiplying by 2 skips the spaces inbetween cells.
     */
    std::string out ( 128, ' ' );
    for ( unsigned i = 0; i < 64; ++i ) 
    { 
        bitboard mask { 1ull << ( i ^ 56 ) };
        if ( occupied_bb () & mask )
        {
            if ( bb ( bbtype::pawn   ) & mask ) out [ i * 2 ] = 'p'; else
            if ( bb ( bbtype::king   ) & mask ) out [ i * 2 ] = 'k'; else
            if ( bb ( bbtype::queen  ) & mask ) out [ i * 2 ] = 'q'; else
            if ( bb ( bbtype::bishop ) & mask ) out [ i * 2 ] = 'b'; else
            if ( bb ( bbtype::knight ) & mask ) out [ i * 2 ] = 'n'; else
            if ( bb ( bbtype::rook   ) & mask ) out [ i * 2 ] = 'r';
            if ( bb ( bbtype::white  ) & mask ) out [ i * 2 ] = std::toupper ( out [ i * 2 ] );
        }
        else out [ i * 2 ] = '.';
        if ( ( i & 7 ) == 7 ) out [ i * 2 + 1 ] = '\n';
    };

    /* Return the formatted string */
    return out;
}