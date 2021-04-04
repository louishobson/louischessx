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
#include <chess/game_controller.h>
#include <chrono>
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

    //cb.get_bb ( chess::pcolor::white, chess::ptype::bishop ).set_value ( 0b000000000000000000000100 );
    //cb.get_bb ( chess::pcolor::white, chess::ptype::pawn ).set_value   ( 0b000110001110011000000000 );
    //cb.get_bb ( chess::pcolor::white, chess::ptype::knight ).set_value ( 0b000001000000000000000000 );
    //cb.get_bb ( chess::pcolor::white ).set_value                       ( 0b000111001110011010011101 );
    //cb.get_bb ( chess::pcolor::black, chess::ptype::pawn ).set_value   ( 0b000000000011110011000011ull << 40 );
    //cb.get_bb ( chess::pcolor::black, chess::ptype::knight ).set_value ( 0b000000000000000000100100ull << 40 );
    //cb.get_bb ( chess::pcolor::black ).set_value                       ( 0b101111010011110011100111ull << 40 );

    chess::game_controller gc;
    
    while ( true )
    {
        std::string cmd; std::getline ( gc.chess_in, cmd ); if ( cmd.back () == '\n' ) cmd.pop_back ();

        gc.handle_command ( cmd );

        if ( cmd.starts_with ( "quit" ) ) return 0;
    }





    ///* Game loop */
    //while ( true )
    //{
    //    /* Print board state */
    //    std::cout << cb.simple_format_board () << "\n";
//
    //    /* Search */
    //    std::atomic_bool end_flag = false;
    //    //auto ab_result_future = cb.alpha_beta_search ( chess::pcolor::white, 8, end_flag );
    //    auto t0 = chess::chess_clock::now ();
    //    auto ab_result = cb.alpha_beta_iterative_deepening ( chess::pcolor::white, { 3, 4, 5, 6, 7, 8, 9, 10 }, true, end_flag, chess::chess_clock::now () + std::chrono::seconds { 15 } );
    //    auto t1 = chess::chess_clock::now ();
//
    //    /* Print the time taken and depth info */
    //    std::cout << "time = " << std::chrono::duration_cast<std::chrono::milliseconds> ( ab_result.duration ).count () << "ms\n";
    //    std::cout << "total time = " << std::chrono::duration_cast<std::chrono::milliseconds> ( t1 - t0 ).count () << "ms\n";
    //    std::cout << "depth = " << ab_result.depth 
    //              << "\nav. q. depth = " << ab_result.av_q_depth 
    //              << "\nnodes visited = " << ab_result.num_nodes 
    //              << "\nq. nodes visited = " << ab_result.num_q_nodes 
    //              << "\nav. moves per node  = " << ab_result.av_moves 
    //              << "\nav. moves per q. node  = " << ab_result.av_q_moves
    //              << "\n\n";
//
    //    /* Break if there were no moves */
    //    if ( ab_result.moves.size () == 0 ) break;
//
    //    /* Print the moves and their values */
    //    for ( const auto& move : ab_result.moves ) std::cout << cb.fide_serialize_move ( move.first ) << ": " << move.second << "\n";
//
    //    /* Print the move and make it */
    //    cb.make_move ( ab_result.moves.front ().first );
//
    //    /* Print board state */
    //    std::cout << "\n" << cb.simple_format_board () << "\n" << cb.fen_serialize_board ( chess::pcolor::black ) << "\n\n";
//
    //    /* Get the user's input */
    //    while ( true )
    //    {
    //        /* Get the move */
    //        std::string move;
    //        std::getline ( std::cin, move );
    //        std::cout << "\n";
//
    //        /* Try to make it. On failure retry */;
    //        try
    //        {
    //            cb.make_move ( cb.fide_deserialize_move ( chess::pcolor::black, move ) );
    //        } catch ( const std::exception& e )
    //        {
    //            std::cout << "Input failed because: " << e.what () << "\n\n";
    //            continue;
    //        }
//
    //        /* Success, so break */
    //        break;
    //    }
    //}


    
    
    /*
    for ( int i = 0; i < 8; ++i )
    {
        std::cout << "{\n";
        for ( int j = 0; j < 64; ++j )
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
    for ( int i = 0; i < 64; ++i )
    {
        chess::bitboard bb { 1ull << i };
        //std::cout << "0x" << std::hex << std::setw ( 16 ) << std::setfill ( '0' ) << bb.queen_all_attack ().get_value ();
        std::cout << bb.queen_all_attack ().format_board () << "\n";
        //std::cout << ( ( i + 1 ) % 8 == 0 ? ",\n" : ", " );
    }
    */

    /*
    std::srand ( std::time ( nullptr ) );
    for ( int i = 0; i < 500000000; ++i )
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

        for ( int j = 0; j < 8; ++j )
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


        cb.get_bb ( chess::pcolor::black ) = cb.bb ( chess::pcolor::white ).rotate_180 ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::pawn   ) = cb.bb ( chess::pcolor::white, chess::ptype::pawn   ).rotate_180 ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::king   ) = cb.bb ( chess::pcolor::white, chess::ptype::king   ).rotate_180 ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::queen  ) = cb.bb ( chess::pcolor::white, chess::ptype::queen  ).rotate_180 ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::knight ) = cb.bb ( chess::pcolor::white, chess::ptype::knight ).rotate_180 ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::bishop ) = cb.bb ( chess::pcolor::white, chess::ptype::bishop ).rotate_180 ();
        cb.get_bb ( chess::pcolor::black, chess::ptype::rook   ) = cb.bb ( chess::pcolor::white, chess::ptype::rook   ).rotate_180 ();

        if ( cb.evaluate ( chess::pcolor::white ) != 0 ) throw;
    }*/
    


    return 0;
}