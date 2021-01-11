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
#include <chess/bitboard.h>



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
    /* Flip the board vertically to access the top rank first */
    bitboard trans = vertical_flip ();

    /* Add to string one byte at a time */
    std::string out;
    for ( unsigned i = 0; i < 64; ++i ) { out += ( trans.test ( i ) ? one : zero ); out += ( ( i + 1 ) & 7 ? " " : "\n" ); };

    /* Return the formatted board */
    return out;
}
