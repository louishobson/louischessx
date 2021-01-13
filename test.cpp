/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * test.cpp
 * 
 * Test source for Chess
 * 
 */



/* INCLUDES */
#include <iostream>
#include <chess/chessboard.h>


/* MAIN */

int main ()
{
    chess::bitboard a { 0xff };
    std::cout << a.fill ( chess::compass::n ).popcount ();
    return 0;
}