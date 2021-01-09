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
#include <chess/chessboard.h>



/* MAIN */

int main ()
{
    unsigned i;
    std::cin >> i;

    chess::bitboard bb { i };
    unsigned j = bb.rotate_90_anticlock ().popcount ();
    //unsigned j = bb.rotate_right ( 5 ).popcount ();

    /* DONE! */

    std::cout << j << std::endl;



    return 0;
}