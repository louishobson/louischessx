/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/bitboard.cpp
 * 
 * implementation of include/chess/bitboard.h
 * 
 */



/* INCLUDES */
#include <chess/bitboard.h>



/* FORMATTING */

/** @name  format_board
 * 
 * @brief  return a string containing newlines for a 8x8 representation of the board
 * @param  zero: the character to insert for 0
 * @param  one:  the character to insert for 1
 * @return the formatted string
 */
std::string chess::bitboard::format_board ( const char zero, const char one ) const
{
    /* flip the board vertically to access the top rank first */
    bitboard trans = vertical_flip ();

    /* add to string one byte at a time */
    std::string out;
    for ( unsigned i = 0; i < 64; ++i ) { out += ( trans.test ( i ) ? one : zero ); out += ( ( i + 1 ) & 7 ? " " : "\n" ); };

    /* return the formatted board */
    return out;
}
