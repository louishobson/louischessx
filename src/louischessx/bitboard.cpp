/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/bitboard.cpp
 * 
 * Implementation of include/chess/bitboard.h
 * 
 */



/* INCLUDES */
#include <louischessx/bitboard.h>



/* FORMATTING */

/** @name  format_board
 * 
 * @brief  Return a string containing newlines for a 8x8 representation of the board
 * @param  zero: The character to insert for 0
 * @param  one:  The character to insert for 1
 * @return The formatted string
 */
std::string chess::bitboard::format_board ( const char zero, const char one ) const
{
    /* Add to string one bit at a time.
     * i ^ 56 changes the endianness of i, such that the top row is read first.
     * Multiplying by 2 skips the spaces inbetween cells.
     */
    std::string out ( 128, ' ' );
    for ( int i = 0; i < 64; ++i ) 
    { 
        out [ i * 2 ] = ( test ( i ^ 56 ) ? one : zero );
        if ( ( i & 7 ) == 7 ) out [ i * 2 + 1 ] = '\n';
    };

    /* Return the formatted board */
    return out;
}
