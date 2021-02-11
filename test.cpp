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
#include <array>
#include <chess/chessboard.h>
#include <iomanip>
#include <iostream>


/* MAIN */

int main ()
{
    
    //unsigned long long i;
    //std::cin >> std::hex >> i;

    chess::chessboard cb;
    //cb.get_bb ( chess::pcolor::white, chess::ptype::pawn   ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::white, chess::ptype::king   ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::white, chess::ptype::queen  ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::white, chess::ptype::knight ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::white, chess::ptype::bishop ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::white, chess::ptype::rook   ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::black, chess::ptype::pawn   ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::black, chess::ptype::king   ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::black, chess::ptype::queen  ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::black, chess::ptype::knight ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::black, chess::ptype::bishop ) |= chess::bitboard { i };
    //cb.get_bb ( chess::pcolor::black, chess::ptype::rook   ) |= chess::bitboard { i };

    std::cout << cb.simple_format_board ();
   
    auto eval = cb.evaluate ( chess::pcolor::white );

    std::cout << eval << std::endl;

    

    /*
    for ( unsigned i = 0; i < 8; ++i )
    {
        std::cout << "{\n";
        for ( unsigned j = 0; j < 64; ++j )
        {
            chess::bitboard bb { 1ull << j };
            std::cout << ( j % 8 == 0 ? "    0x" : "0x" );
            std::cout << std::hex << std::setw ( 16 ) << std::setfill ( '0' ) << bb.span ( static_cast<chess::compass> ( i ) ).get_value ();
            std::cout << ( ( j + 1 ) % 8 == 0 ? ",\n" : ", " );
        }
        std::cout << "},\n";
    }
    */

   /*
   for ( unsigned i = 0; i < 64; ++i )
   {
       chess::bitboard bb { 1ull << i };
       //std::cout << "0x" << std::hex << std::setw ( 16 ) << std::setfill ( '0' ) << bb.queen_all_attack ().get_value ();
       std::cout << bb.queen_all_attack ().format_board () << "\n";
       //std::cout << ( ( i + 1 ) % 8 == 0 ? ",\n" : ", " );
   }
   */

    return 0;
}