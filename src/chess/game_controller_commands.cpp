/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/game_controller_commands.cpp
 * 
 * Implementation of command handling methods in include/chess/game_controller.h
 * 
 */



/* INCLUDES */
#include <chess/game_controller.h>



/* COMMAND HANDLING METHODS */

/** @name  handle_command
 * 
 * @brief  Take a command and fully handle it before returning.
 *         Note that a command may start a new thread and return immediately, this being considered having handled the command.
 * @param  cmd: The command to handle, with its arguments separated by spaces, without the newline.
 * @return True if the command was handled, false if it was unrecognised (thus ignored).
 */
bool chess::game_controller::handle_command ( const std::string& cmd )
{
    /* The following if-else chain detects commands and handles them */

    /** @name  xboard
     * 
     * @brief  Tells this engine that xboard is sending commands.
     *         The next command must be protover with a version of at least 2.
     * @return Respond with feature requests and check they are accepted.
     */
    if ( cmd.starts_with ( "xboard" ) )
    {
        /* Wait for the protover command */
        std::string next_cmd; std::getline ( chess_in, next_cmd );

        /* Check that it is the protover command, and that this is version 2 */
        if ( !next_cmd.starts_with ( "protover " ) || std::stoi ( next_cmd.substr ( 9 ) ) < 2 ) throw chess_input_error { "Did not recieve valid protover command after xboard" };

        /* Iterate through features, send them, and make sure they're accepted */
        for ( const char * feature : feature_requests )
        {
            /* Send the feature */
            chess_out << feature << std::endl;

            /* Wait for the feature response */
            std::string feature_response; std::getline ( chess_in, feature_response );

            /* Throw if the response is not accepted */
            if ( !feature_response.starts_with ( "accepted" ) ) throw chess_input_error { "Feature requests rejected by the interface." };
        }
    } else

    /** @name  new
     * 
     * @brief  Tells this engine to reset the board to its default state.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "new" ) )
    {
        /* Cancel any ongoing search */
        stop_precomputation ();

        /* Reset the board and next player to move */
        game_cb.reset_to_initial (); next_pc = pcolor::white;
    } else

    /** @name  variant
     * 
     * @brief  Gives what variant of chess is being played. Always throws, since variants are unsupported.
     */
    if ( cmd.starts_with ( "variant" ) ) throw chess_input_error { "Cannot handle 'variant' command." }; else

    /** @name  quit
     * 
     * @brief  Supplied when the engine should quit immediately. Simply cancels ongoing search.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "quit" ) ) stop_precomputation (); else

    /** @name  force
     * 
     * @brief  Put the engine into force mode, such that it observes moves and checks for legality, but does not move itself.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "force" ) )
    {
        /* Stop precomputation */
        stop_precomputation ();

        /* Set the color that the computer plays to no_color */
        computer_pc = pcolor::no_piece;
    } else

    /** @name  go
     * 
     * @brief  Leave force mode (if active) and make the engine play as the color who's turn it is now.
     * @return Move command, and/or result if a checkmate occurs.
     */
    if ( cmd.starts_with ( "go" ) )
    {
        /* Stop precomputation */
        stop_precomputation ();

        /* Set the computer color to the currently active color */
        computer_pc = next_pc;

        /* Start a search and get the result */
        chessboard::ab_result_t ab_result = start_search ( game_cb, computer_pc, true )->ab_result_future.get ();
        
        /* Output the move, if any */
        output_move ( ab_result );
    } else

    /** @name  playother
     * 
     * @brief  Leave force move (if active) and make the engine play as the other color to the one who's turn it is now.
     *         Hence start pondering.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "playother" ) )
    {
        /* Stop precomputation */
        stop_precomputation ();

        /* Set the computer color to the color that's not currently active */
        computer_pc = other_color ( next_pc );

        /* Start pondering */
        start_precomputation ( computer_pc );
    } else

    /** @name  move MOVE
     * 
     * @brief  The move that the opponent has made.
     * @param  MOVE: The move in standard algebraic notation.
     * @return Move command, and/or result if a checkmate occurs.
     */
    if ( cmd.starts_with ( "usermove" ) )
    {
        /* Try all of the below */
        try
        {
            /* Check that it is not the computer's move */
            if ( next_pc == computer_pc ) { throw chess_input_error { "Error (unexpected move): it is not your turn!" }; return false; }

            /* Try to decode the move description, and make the move */
            const move_t move = game_cb.fide_deserialize_move ( next_pc, cmd.substr ( 9 ) );

            /* Try to make the move and swap next_pc */
            game_cb.make_move ( move ); next_pc = other_color ( next_pc );

            /* If not in forced mode, respond to the usermove */
            if ( computer_pc == next_pc )
            {
                /* Stop any precomputation */
                search_data_it_t search_data_it = stop_precomputation ( move );

                /* Start the correct search if had not already been started */
                if ( search_data_it == active_searches.end () ) search_data_it = start_search ( game_cb, computer_pc, true );

                /* Get the result of the search. Wait slightly longer than the max response duration, to stop the timeout from just missing the end of the search. */
                if ( search_data_it->ab_result_future.wait_for ( max_response_duration + std::chrono::milliseconds { 500 } ) == std::future_status::timeout ) search_data_it->end_flag = true;
                chessboard::ab_result_t ab_result = search_data_it->ab_result_future.get ();
                
                /* Output the move, if any */
                output_move ( ab_result );
            }
        } 
        
        /* Catch an exception from the above and output an error */
        catch ( const std::exception& e ) { chess_out << "Error (failed usermove): " << e.what () << std::endl; return false; }
    } else

    /** @name  ping N
     * 
     * @brief  Ping pong command.
     * @param  N: Some integer
     * @return pong N
     */
    if ( cmd.starts_with ( "ping" ) )
    {
        /* Output pong N */
        chess_out << "pong " << cmd.substr ( 5 ) << std::endl;
    } else

    /** @name  draw
     * 
     * @brief  The engine is receiving a draw offer.
     * @return offer draw if the engine is losing.
     */
    if ( cmd.starts_with ( "draw" ) )
    {
        /* If in force mode, return an error */
        if ( computer_pc == pcolor::no_piece ) { chess_out << "Error (invalid draw): offered draw while in forced mode." << std::endl; return false; }

        /* If the evaluation of game_cb is <= -100 for the computer, then accept */
        if ( game_cb.evaluate ( computer_pc ) <= -100 ) chess_out << "offer draw" << std::endl;
    } else

    /** @name  setboard FEN
     * 
     * @brief  Set the board to a specific layout.
     * @param  FEN: The Forsyth-Edwards notation for the game state.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "setboard" ) )
    {
        /* Stop any precomputation */
        stop_precomputation ();

        /* Encase the deserialization in a try block */
        try
        {
            /* Set the game state */
            next_pc = game_cb.fen_deserialize_board ( cmd.substr ( 9 ) );
        }
        catch ( const std::exception& e )
        {
            /* Catch and output an error */
            chess_out << "Error (failed setboard): " << e.what () << std::endl; return false;
        }
    }

    return true;
}



/** @name  output_move
 * 
 * @brief  Takes an ab_result and outputs the best move, if there is one, as well as a result if the game has ended.
 *         Also starts precomputation if not in force mode.
 * @param  ab_result: The result of the search on this state.
 * @return void
 */
void chess::game_controller::output_move ( const chessboard::ab_result_t& ab_result )
{
    /* Check that it is the computer's move */
    if ( next_pc != computer_pc ) { throw chess_input_error { "Error (unexpected move): computer tried to output a move when it's not its turn" }; return; } 

    /* If there are any moves the computer can make */
    if ( ab_result.moves.size () ) 
    {
        /* Output the move */
        chess_out << "move " << game_cb.fide_serialize_move ( ab_result.moves.front ().first ) << std::endl;

        /* Swap the next color */
        next_pc = other_color ( next_pc ); 

        /* If is a win, stalemate or draw, output a result */
        if ( ab_result.moves.front ().first.checkmate ) chess_out << ( computer_pc == pcolor::white ? "1-0 {White mates}" : "0-1 {Black mates}" ) << std::endl; else
        if ( ab_result.moves.front ().first.stalemate ) chess_out << "1/2-1/2 {Stalemate}" << std::endl; else
        if ( ab_result.moves.front ().first.draw      ) chess_out << "1/2-1/2 {Draw by repetition}" << std::endl; else

        /* Else, since the game has not ended, start precomputation */
        start_precomputation ( next_pc );
    }

    /* Else if there are no moves, it will be a checkmate or draw */
    else
    {
        /* Evaluate the current position */
        const int value = game_cb.evaluate ( computer_pc );

        /* Check if it is a checkmate */
        if ( value == -10000 ) chess_out << ( computer_pc == pcolor::white ? "0-1 {Black mates}" : "1-0 {White mates}" ) << std::endl; else

        /* Else if is a stalemate */
        if ( value == 0 ) chess_out << "1/2-1/2 {Stalemate}" << std::endl;

        /* Else it must be a draw */
        else chess_out << "1/2-1/2 {Draw by repetition}" << std::endl;
    }
}