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
#include <louischessx/game_controller.h>



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
        if ( !next_cmd.starts_with ( "protover " ) || next_cmd.size () < 10 || safe_stoi ( next_cmd.substr ( 9 ) ) < 2 ) throw chess_input_error { "Did not recieve valid protover command after xboard" };

        /* Send feature requests */ 
        write_chess_out ( "feature done=0"          ); /* Pause timeout on feature requests */
        write_chess_out ( "feature ping=1"          ); /* Allow the ping command */
        write_chess_out ( "feature setboard=1"      ); /* Allow the setboard command */
        write_chess_out ( "feature playother=1"     ); /* Allow the playother command */
        write_chess_out ( "feature san=1"           ); /* Force standard algebraic notation for moves */
        write_chess_out ( "feature usermove=1"      ); /* Force user moves to be given only with the usermove command */
        write_chess_out ( "feature time=1"          ); /* Set time updates to be ignored */
        write_chess_out ( "feature sigint=0"        ); /* Stop interrupt signals from being sent */
        write_chess_out ( "feature sigterm=0"       ); /* Stop terminate signals from being send */
        write_chess_out ( "feature myname=LouisBot" ); /* Name this engine */
        write_chess_out ( "feature colors=0"        ); /* Don't send the 'white' or 'black' commands */
        write_chess_out ( "feature done=1"          ); /* End of features */
    } else

    /** @name  accepted, rejected
     * 
     * @brief  Tells the engine whether features were accepted or rejected. Ignore these commands.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "accepted" ) || cmd.starts_with ( "rejected" ) ); else

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

        /* Change to normal mode */
        mode = computer_mode_t::normal;

        /* Set the next color to white, the computer to black */
        next_pc = pcolor::white; computer_pc = pcolor::black;

        /* Set to not be playing a computer */
        opponent_is_computer = false;

        /* Reset the clocks */
        computer_clock = opponent_clock = time_base;

        /* Start time control */
        start_time_control ();

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

        /* Enter force mode */
        mode = computer_mode_t::force;
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

        /* Set to normal mode */
        mode = computer_mode_t::normal;

        /* Set the computer color to the currently active color */
        computer_pc = next_pc;

        /* Start time control */
        start_time_control ();

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

        /* Set to normal mode */
        mode = computer_mode_t::normal;

        /* Set the computer color to the color that's not currently active */
        computer_pc = other_color ( next_pc );

        /* Start time control */
        start_time_control ();

        /* Start pondering */
        start_precomputation ();
    } else

    /** @name  level CONTROL BASE INC
     * 
     * @brief  Set a classical or incremental time control. Which one is being set is given by the CONTROL parameter (see below).
     * @param  CONTROL: For classical, the number of moves per time control. For incremental, always 0.
     * @param  BASE: For classical, the size of the time control. For incremental, the time each player starts with. Defaults to minutes unless in MIN:SECS format.
     * @param  INC: For classical, always 0. For incremental, the time increment per move by each player. Defaults to seconds unless in MIN:SECS format.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "level " ) )
    {
        /* Stop pondering */
        stop_precomputation ();

        /* Look for the parameters */
        std::smatch time_match;
        if ( !std::regex_match ( cmd, time_match, std::regex { "^level (\\d+) (\\d+)(?::(\\d+))? (?:(\\d+):)?(\\d+)$" } ) ) throw chess_input_error { "Could not format paramaters." };

        /* Set the paramaters */
        moves_per_control = safe_stoi ( time_match.str ( 1 ) );
        time_base         = std::chrono::minutes { safe_stoi ( time_match.str ( 2 ) ) } + std::chrono::seconds { time_match.length ( 3 ) ? safe_stoi ( time_match.str ( 3 ) ) : 0 };
        time_increase     = std::chrono::seconds { safe_stoi ( time_match.str ( 5 ) ) } + std::chrono::minutes { time_match.length ( 4 ) ? safe_stoi ( time_match.str ( 4 ) ) : 0 };
        clock_type        = ( moves_per_control ? clock_type_t::classical : clock_type_t::incremental );
    
        /* Set the remaining time to the time base */
        computer_clock = opponent_clock = time_base;

        /* If in normal mode, restart pondering */
        if ( mode == computer_mode_t::normal ) { start_time_control (); start_precomputation (); }
    } else

    /** @name  st TIME
     * 
     * @brief  Fixed max time controll.
     * @param  TIME: The amount of time for each move. Defaults to seconds unless in MIN:SECS format.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "st " ) )
    {
        /* Stop pondering */
        stop_precomputation ();

        /* Look for the time part of the time control. Throw if ill-formed */
        std::smatch time_match;
        if ( !std::regex_search ( cmd, time_match, std::regex { "^st (?:(\\d+):)?(\\d+)$" } ) ) throw chess_input_error { "Could not format time parameter." };

        /* Set the required paramaters */
        clock_type = clock_type_t::fixed_max;
        time_base  = std::chrono::seconds { safe_stoi ( time_match.str ( 2 ) ) } + std::chrono::minutes { time_match.length ( 1 ) ? safe_stoi ( time_match.str ( 1 ) ) : 0 };
    
        /* Set the remaining time to the time base */
        computer_clock = opponent_clock = time_base;

        /* If in normal mode, restart pondering */
        if ( mode == computer_mode_t::normal ) { start_time_control (); start_precomputation (); } 
    } else

    /** @name  time N
     * 
     * @brief  Set the computer clock to have N centiseconds remaining. Used to synchronize clocks.
     * @param  N: The time remaining, in centiseconds.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "time " ) )
    {
        /* Set the clock, if the clock type is not fixed max */
        if ( clock_type != clock_type_t::fixed_max ) computer_clock = std::chrono::duration<chess_clock::rep, std::centi> { safe_stoi ( cmd.substr ( 5 ) ) };
    } else

    /** @name  otim N
     * 
     * @brief  Set the opponent clock to have N centiseconds remaining. Used to synchronize clocks.
     * @param  N: The time remaining, in centiseconds.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "otim " ) )
    {
        /* Ignore if the clock type is fixed max */
        if ( clock_type != clock_type_t::fixed_max )
        {
            /* Get the time in centiseconds */
            const std::chrono::duration<chess_clock::rep, std::centi> new_opponent_clock { safe_stoi ( cmd.substr ( 5 ) ) };

            /* If in normal mode, store it in opponent_sync_clock, otherwise set the clock immediately */
            if ( mode == computer_mode_t::normal ) opponent_sync_clock = new_opponent_clock; else opponent_clock = new_opponent_clock; 
        }
    } else

    /** @name  usermove MOVE
     * 
     * @brief  The move that the opponent has made.
     * @param  MOVE: The move in standard algebraic notation.
     * @return Move command, and/or result if a checkmate occurs.
     */
    if ( cmd.starts_with ( "usermove " ) )
    {
        /* Check that there is a move supplied */
        if ( cmd.size () < 10 ) throw chess_input_error { "Move not supplied with usermove." };

        /* Try to decode the move description */
        const move_t move = game_cb.fide_deserialize_move ( next_pc, cmd.substr ( 9 ) );

        /* Try to make the move */
        game_cb.make_move ( move );
        
        /* Stop time control and tell the user if time has run out */
        if ( !end_time_control () ) write_chess_out ( "telluser You've run out of time!" );

        /* Switch next color */
        next_pc = other_color ( next_pc );

        /* Start time control */
        start_time_control ();

        /* If in normal mode, respond to the move */
        if ( mode == computer_mode_t::normal )
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
        /* Output pong N */
        write_chess_out ( "pong ", cmd.substr ( 5 ) );
    } else

    /** @name  draw
     * 
     * @brief  The engine is receiving a draw offer.
     * @return offer draw if the engine is losing.
     */
    if ( cmd.starts_with ( "draw" ) )
    {
        /* If not in normal mode, throw an input error */
        if ( mode != computer_mode_t::normal ) throw chess_input_error { "Offered draw while in force mode." };

        /* If latest_best_value <= draw_offer_acceptance_value for the computer, then accept */
        if ( latest_best_value <= draw_offer_acceptance_value ) write_chess_out ( "offer draw" );
    } else

    /** @name  result RESULT
     * 
     * @brief  The game has ended, according to xboard.
     * @param  result: The result of the game.
     * @return Nothing. Ignore the command.
     */
    if ( cmd.starts_with ( "result" ) ); else

    /** @name  setboard FEN
     * 
     * @brief  Set the board to a specific layout.
     * @param  FEN: The Forsyth-Edwards notation for the game state.
     * @return Nothing.
     */
    if ( cmd.starts_with ( "setboard " ) )
    {
        /* Throw if is not in force mode */
        if ( mode != computer_mode_t::force ) throw chess_input_error { "Recieved 'setboard' command when the computer is not in force mode." };

        /* Stop any precomputation */
        stop_precomputation ();

        /* Set the game state */
        next_pc = game_cb.fen_deserialize_board_keep_history ( cmd.substr ( 9 ) );

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
        /* Throw if is not in force mode */
        if ( mode != computer_mode_t::force ) throw chess_input_error { "Recieved 'undo' command when the computer is not in force mode." };

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

        /* Undo the last two moves */
        game_cb.unmake_move (); game_cb.unmake_move ();

        /* Restart the clock */
        start_time_control ();

        /* Restart precomputation */
        start_precomputation ();
    } else

    /** @name  computer
     * 
     * @brief  Set the opponent to be a computer
     * @return Nothing.
     */
    if ( cmd.starts_with ( "computer" ) ) opponent_is_computer = true;

    /* Else the command is unrecognised, so throw an error */
    else throw chess_input_error { "Unknown command." };

    /* Command handled, so return true */
    return true;
} 

/* Catch an input error */
catch ( const chess_input_error& e )
{
    /* If the command was a usermove, send an illegal move command */
    if ( cmd.starts_with ( "usermove" ) ) write_chess_out ( "Illegal move (", e.what (), "): ", cmd );

    /* Else output as a normal error */
    else write_chess_out ( "Error (", e.what (), "): ", cmd );

    /* Return false */
    return false;
} 

/* Catch an internal error */
catch ( const chess_internal_error& e )
{
    /* Output the error to the user */
    write_chess_out ( "tellusererror (", e.what (), "): ", cmd );

    /* Resign */
    write_chess_out ( "resign" );

    /* Rethrow */
    throw;
}



/** @name  safe_stoi
 * 
 * @brief  Call std::stoi, but rethrow an exception as a chess_input_error.
 * @param  str: The string to convert
 * @return The converted integer.
 */
int chess::game_controller::safe_stoi ( const std::string& str ) const try
{
    /* Return std::stoi of str */
    return std::stoi ( str );
} 

/* Catch an exception */
catch ( const std::exception& e )
{
    /* Rethrow */
    throw chess_input_error { "Failed to convert string to an inteter." };
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
    /* Check is in normal mode */
    if ( mode != computer_mode_t::normal ) throw chess_internal_error { "Computer tried to output a move while in force mode." };

    /* Check that it is the computer's move */
    if ( next_pc != computer_pc ) throw chess_internal_error { "Computer tried to output a move when it's not its turn." }; 

    /* If the depth is 0, the ab_result is invalid, so throw */
    if ( ab_result.depth == 0 ) throw chess_internal_error { "Invalid ab_result in make_and_output_move ()." };

    /* If there are any moves the computer can make */
    if ( ab_result.moves.size () ) 
    {
        /* Output the move */
        write_chess_out ( "move ", game_cb.fide_serialize_move ( ab_result.moves.front ().first ) );

        /* Output information about the last move */
        write_chess_out ( "# duration = ", std::chrono::duration_cast<std::chrono::milliseconds> ( ab_result.duration ).count (), "ms" );
        write_chess_out ( "# depth = ",                  ab_result.depth        );
        write_chess_out ( "# av. q. depth = ",           ab_result.av_q_depth   );
        write_chess_out ( "# nodes visited = ",          ab_result.num_nodes    );
        write_chess_out ( "# q. nodes visited = ",       ab_result.num_q_nodes  );
        write_chess_out ( "# av. moves per node  = ",    ab_result.av_moves     );
        write_chess_out ( "# av. moves per q. node  = ", ab_result.av_q_moves   );

        /* Make the move */
        game_cb.make_move ( ab_result.moves.front ().first );

        /* End time control */
        end_time_control ();

        /* Set the latest best value */
        latest_best_value = ab_result.moves.front ().second;

        /* Set the cumulative ttable */
        cumulative_ttable = game_cb.purge_ttable ( std::move ( ab_result.ttable ), ttable_min_bk_depth );

        /* Swap the next color */
        next_pc = other_color ( next_pc ); 

        /* If is a win, stalemate or draw, output a result */
        if ( ab_result.moves.front ().first.checkmate ) write_chess_out ( computer_pc == pcolor::white ? "1-0 {White mates}" : "0-1 {Black mates}" ); else
        if ( ab_result.moves.front ().first.stalemate ) write_chess_out ( "1/2-1/2 {Stalemate}" ); else
        if ( ab_result.moves.front ().first.draw      ) write_chess_out ( "1/2-1/2 {Draw by repetition}" ); else

        /* Else, since the game has not ended, start time controls and precomputation */
        { start_time_control (); start_precomputation (); }
    }

    /* Else if there are no moves, it will be a checkmate or draw */
    else
    {
        /* Get if the computer is in check or has possible moves */
        const chessboard::check_info_t computer_check_info = game_cb.get_check_info ( computer_pc );
        const bool computer_has_mobility = game_cb.has_mobility ( computer_pc, computer_check_info );

        /* If computer is in check with no mobility, this state is a checkmate */
        if ( computer_check_info.check_count && !computer_has_mobility ) write_chess_out ( computer_pc == pcolor::white ? "0-1 {Black mates}" : "1-0 {White mates}" ); else

        /* Else if the computer is not in check and doesn't have any mobility, then this is a stalemate */
        if ( !computer_check_info.check_count && !computer_has_mobility ) write_chess_out ( "1/2-1/2 {Stalemate}" ); else

        /* Else check if this is a draw state */
        if ( game_cb.is_draw_state () ) write_chess_out ( "1/2-1/2 {Draw by repetition}" ); else

        /* Else the ab_result has no moves for an unknown reason, so produce an internal error */
        throw chess_internal_error { "Cannot discern why ab_result has no possible moves." };
    }
}