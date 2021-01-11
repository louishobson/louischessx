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
    unsigned long long i;
    std::cin >> i; 

    chess::bitboard a { i };

    /* DONE! */

    //std::cout << a.format_board () << std::endl << a.vertical_flip ().format_board () << std::endl;
    std::cout << a.occluded_fill_ne ().format_board ();



    return 0;
}