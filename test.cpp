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
    //unsigned long long i;
    //std::cin >> i; 

    chess::bitboard a { 0x1 };

    /* DONE! */

    //std::cout << a.format_board () << std::endl << a.vertical_flip ().format_board () << std::endl;
    std::cout << a.span_ne ( ~chess::bitboard { 255 }.rotate_90_aclock () ).popcount ();



    return 0;
}