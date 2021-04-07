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
bool chess::game_controller::handle_command ( const std::string& cmd ) try
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
        /* Wait for the protover command and check for at least version 2 */
        std::string next_cmd; std::getline ( chess_in, next_cmd );
        if ( !next_cmd.starts_with ( "protover " ) || next_cmd.size () < 10 || std::stoi ( next_cmd.substr ( 9 ) ) < 2 ) throw chess_input_error { "Did not recieve valid protover command after xboard" };

        /* Send feature requests */
        chess_out << "feature done=0\n"          /* Pause timeout on feature requests */
                  << "feature ping=1\n"          /* Allow the ping command */
                  << "feature setboard=1\n"      /* Allow the setboard command */
                  << "feature playother=1\n"     /* Allow the playother command */
                  << "feature san=1\n"           /* Force standard algebraic notation for moves */
                  << "feature usermove=1\n"      /* Force user moves to be given only with the usermove command */
                  << "feature time=0\n"          /* Set time updates to be ignored */
                  << "feature sigint=0\n"        /* Stop interrupt signals from being sent */
                  << "feature sigterm=0\n"       /* Stop terminate signals from being send */
                  << "feature myname=LouisBot\n" /* Name this engine */
                  << "feature colors=0\n"        /* Don't send the 'white' or 'black' commands */
                  << "feature done=1\n"          /* End of features */
                  << std::flush;
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

        /* Reset the board and clear the cumulative ttable */
        game_cb.reset_to_initial (); cumulative_ttable.clear ();

        /* Set the next color to white, the computer to black */
        next_pc = pcolor::white; computer_pc = pcolor::black; 

        /* Start precomputation */
        start_precomputation ();
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
        chessboard::ab_result_t ab_result = start_search ( game_cb, computer_pc, move_t {}, cumulative_ttable, true )->ab_result_future.get ();
        
        /* Output the move, if any */
        make_and_output_move ( ab_result );
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
        start_precomputation ();
    } else

    /** @name  move MOVE
     * 
     * @brief  The move that the opponent has made.
     * @param  MOVE: The move in standard algebraic notation.
     * @return Move command, and/or result if a checkmate occurs.
     */
    if ( cmd.starts_with ( "usermove " ) )
    {
        /* Check that it is not the computer's move */
        if ( next_pc == computer_pc ) throw chess_input_error { "User tried to move on computer's turn." };

        /* Check that there is a move supplied */
        if ( cmd.size () < 10 ) throw chess_input_error { "Move not supplied with usermove." };

        /* Try to decode the move description */
        const move_t move = game_cb.fide_deserialize_move ( next_pc, cmd.substr ( 9 ) );

        /* Try to make the move and swap next_pc */
        game_cb.make_move ( move ); next_pc = other_color ( next_pc );

        /* If not in forced mode, respond to the usermove */
        if ( computer_pc == next_pc )
        {
            /* Stop any precomputation */
            search_data_it_t search_data_it = stop_precomputation ( move );

            /* Start the correct search if had not already been started */
            if ( search_data_it == active_searches.end () ) search_data_it = start_search ( game_cb, computer_pc, move, game_cb.purge_ttable ( cumulative_ttable, ttable_min_bk_depth ), true );

            /* Get the result of the search. Wait slightly longer than the max response duration, to stop the timeout from just missing the end of the search. */
            if ( search_data_it->ab_result_future.wait_for ( max_response_duration ) == std::future_status::timeout ) search_data_it->end_flag = true;
            chessboard::ab_result_t ab_result = search_data_it->ab_result_future.get ();
            
            /* Output the move, if any */
            make_and_output_move ( ab_result );
        }
    } else

    /** @name  ping N
     * 
     * @brief  Ping pong command.
     * @param  N: Some integer
     * @return pong N
     */
    if ( cmd.starts_with ( "ping " ) )
    {
        /* Check that there is an N present */
        if ( cmd.size () < 6 ) throw chess_input_error { "No argument supplied with ping." };

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
        /* If in force mode, throw an input error */
        if ( computer_pc == pcolor::no_piece ) throw chess_input_error { "Offered draw while in force mode." };

        /* If latest_best_value <= draw_offer_acceptance_value for the computer, then accept */
        if ( latest_best_value <= draw_offer_acceptance_value ) chess_out << "offer draw" << std::endl;
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

        /* Set the game state */
        next_pc = game_cb.fen_deserialize_board ( cmd.substr ( 9 ) );

        /* Clear the transposition table */
        cumulative_ttable.clear ();
    } else

    /** @name  undo
     * 
     * @brief  Undo the last move. Should be in force mode.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "undo" ) )
    {
        /* Check is in force mode */
        if ( computer_pc != pcolor::no_piece ) throw chess_input_error { "Recieved 'undo' command when the computer is not in force mode." };

        /* Undo the last move */
        game_cb.unmake_move ();
    } else

    /** @name  remove
     * 
     * @brief  Undo the last two moves. Should not be the computer's turn.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "remove" ) )
    {
        /* Stop precomputation */
        stop_precomputation ();

        /* Throw if it is the computers turn */
        if ( next_pc == computer_pc ) throw chess_input_error { "Recieved 'remove' command when it is the computer's turn." };

        /* Undo the last two moves */
        game_cb.unmake_move (); game_cb.unmake_move ();

        /* Restart precomputation */
        start_precomputation ();
    }

    /* Command handled, so return true */
    return true;
} 

/* Catch an input error */
catch ( const chess_input_error& e )
{
    /* If the command was a usermove, send an illegal move command */
    if ( cmd.starts_with ( "usermove" ) ) chess_out << "Illegal move (" << e.what () << "): " << cmd << std::endl;

    /* Else output as a normal error */
    else chess_error << "Error (" << e.what () << "): " << cmd << std::endl;

    /* Return false */
    return false;
} 

/* Catch an internal error */
catch ( const chess_internal_error& e )
{
    /* Output the error to the user */
    chess_error << "tellusererror (" << e.what () << "): " << cmd << std::endl;

    /* Resign */
    chess_out << "resign" << std::endl;

    /* Rethrow */
    throw;
}



/** @name  make_and_output_move
 * 
 * @brief  Takes an ab_result and performs, then outputs the best move, if there is one, as well as a result if the game has ended.
 *         Also starts precomputation if not in force mode.
 * @param  ab_result: The result of the search on this state.
 * @return void
 */
void chess::game_controller::make_and_output_move ( chessboard::ab_result_t& ab_result )
{
    /* Check that it is the computer's move */
    if ( next_pc != computer_pc ) throw chess_internal_error { "Computer tried to output a move when it's not its turn." }; 

    /* If the depth is 0, the ab_result is invalid, so throw */
    if ( ab_result.depth == 0 ) throw chess_internal_error { "Invalid ab_result in make_and_output_move ()." };

    /* If there are any moves the computer can make */
    if ( ab_result.moves.size () ) 
    {
        /* Output the move */
        chess_out << "move " << game_cb.fide_serialize_move ( ab_result.moves.front ().first ) << std::endl;

        /* Output information about the last move */
        chess_out << "# duration = " << std::chrono::duration_cast<std::chrono::milliseconds> ( ab_result.duration ).count () << "ms"
                  << "\n# depth = " << ab_result.depth 
                  << "\n# av. q. depth = " << ab_result.av_q_depth 
                  << "\n# nodes visited = " << ab_result.num_nodes 
                  << "\n# q. nodes visited = " << ab_result.num_q_nodes 
                  << "\n# av. moves per node  = " << ab_result.av_moves 
                  << "\n# av. moves per q. node  = " << ab_result.av_q_moves
                  << std::endl;

        /* Make the move */
        game_cb.make_move ( ab_result.moves.front ().first );

        /* Set the latest best value */
        latest_best_value = ab_result.moves.front ().second;

        /* Set the cumulative ttable */
        cumulative_ttable = game_cb.purge_ttable ( std::move ( ab_result.ttable ), ttable_min_bk_depth );

        /* Swap the next color */
        next_pc = other_color ( next_pc ); 

        /* If is a win, stalemate or draw, output a result */
        if ( ab_result.moves.front ().first.checkmate ) chess_out << ( computer_pc == pcolor::white ? "1-0 {White mates}" : "0-1 {Black mates}" ) << std::endl; else
        if ( ab_result.moves.front ().first.stalemate ) chess_out << "1/2-1/2 {Stalemate}" << std::endl; else
        if ( ab_result.moves.front ().first.draw      ) chess_out << "1/2-1/2 {Draw by repetition}" << std::endl; else

        /* Else, since the game has not ended, start precomputation */
        start_precomputation ();
    }

    /* Else if there are no moves, it will be a checkmate or draw */
    else
    {
        /* Get if the computer is in check or has possible moves */
        const chessboard::check_info_t computer_check_info = game_cb.get_check_info ( computer_pc );
        const bool computer_has_mobility = game_cb.has_mobility ( computer_pc, computer_check_info );

        /* If computer is in check with no mobility, this state is a checkmate */
        if ( computer_check_info.check_count && !computer_has_mobility ) chess_out << ( computer_pc == pcolor::white ? "0-1 {Black mates}" : "1-0 {White mates}" ) << std::endl; else

        /* Else if the computer is not in check and doesn't have any mobility, then this is a stalemate */
        if ( !computer_check_info.check_count && !computer_has_mobility ) chess_out << "1/2-1/2 {Stalemate}" << std::endl; else

        /* Else check if this is a draw state */
        if ( game_cb.is_draw_state () ) chess_out << "1/2-1/2 {Draw by repetition}" << std::endl; else

        /* Else the ab_result has no moves for an unknown reason, so produce an internal error */
        throw chess_internal_error { "Cannot discern why ab_result has no possible moves." };
    }
}