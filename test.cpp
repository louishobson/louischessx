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
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>


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

    //cb.get_bb ( chess::pcolor::white, chess::ptype::pawn ).set_value ( 0b0111111000000000 );
    //cb.get_bb ( chess::pcolor::white ).set_value ( 0b0111111011111111 );
    //cb.get_bb ( chess::pcolor::black, chess::ptype::pawn ).set_value ( 0b0000000001111110ull << 48 );
    //cb.get_bb ( chess::pcolor::black ).set_value ( 0b1111111101111110ull << 48 );

    std::cout << cb.simple_format_board () << "\n";
   
    auto eval = cb.evaluate ( chess::pcolor::white );
    auto val = cb.alpha_beta_search ( chess::pcolor::white, 11 );

    std::cout << "\n" << cb.simple_format_board () << "\n";

    std::cout << val << std::endl;

    
    
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

    /*
    std::srand ( std::time ( nullptr ) );
    for ( unsigned i = 0; i < 500000000; ++i )
    {
        chess::chessboard cb;

        cb.get_bb ( chess::pcolor::white ).empty ();
        cb.get_bb ( chess::pcolor::white, chess::ptype::pawn   ).empty ();
        cb.get_bb ( chess::pcolor::white, chess::ptype::king   ).empty ();
        cb.get_bb ( chess::pcolor::white, chess::ptype::queen  ).empty ();
        cb.get_bb ( chess::pcolor::white, chess::ptype::knight ).empty ();
        cb.get_bb ( chess::pcolor::white, chess::ptype::bishop ).empty ();
        cb.get_bb ( chess::pcolor::white, chess::ptype::rook   ).empty ();
        cb.get_bb ( chess::pcolor::black ).empty ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::pawn   ).empty ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::king   ).empty ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::queen  ).empty ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::knight ).empty ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::bishop ).empty ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::rook   ).empty ();

        int random;

        for ( unsigned j = 0; j < 8; ++j )
        {
            random = std::rand () % 32;
            cb.get_bb ( chess::pcolor::white ).set ( random );
            cb.get_bb ( chess::pcolor::white, chess::ptype::pawn ).set ( random );
        }


        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::bishop ).set ( random );

        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::bishop ).set ( random );


        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::rook ).set ( random );

        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::rook ).set ( random );


        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::knight ).set ( random );

        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::knight ).set ( random );


        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::queen ).set ( random );

        random = std::rand () % 32;
        cb.get_bb ( chess::pcolor::white ).set ( random );
        cb.get_bb ( chess::pcolor::white, chess::ptype::king ).set ( random );


        cb.get_bb ( chess::pcolor::black ) = cb.bb ( chess::pcolor::white ).vertical_flip ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::pawn   ) = cb.bb ( chess::pcolor::white, chess::ptype::pawn   ).vertical_flip ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::king   ) = cb.bb ( chess::pcolor::white, chess::ptype::king   ).vertical_flip ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::queen  ) = cb.bb ( chess::pcolor::white, chess::ptype::queen  ).vertical_flip ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::knight ) = cb.bb ( chess::pcolor::white, chess::ptype::knight ).vertical_flip ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::bishop ) = cb.bb ( chess::pcolor::white, chess::ptype::bishop ).vertical_flip ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::rook   ) = cb.bb ( chess::pcolor::white, chess::ptype::rook   ).vertical_flip ();


        if ( cb.is_in_check ( chess::pcolor::white ) ) continue;


        if ( cb.evaluate ( chess::pcolor::white ) != 0 ) throw;

    }
    */


    return 0;
}