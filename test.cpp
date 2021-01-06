/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * test.cpp
 * 
 * test source for Chess
 * 
 */



/* INCLUDES */
#include <iostream>
#include <chess/bitboard.h>


/* MAIN */

int main ()
{
    std::cout << "Hello, world!" << std::endl;

    std::cout << chess::masks::light_squares.popcount () << std::endl;

    return 0;
}