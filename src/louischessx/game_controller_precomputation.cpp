/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/game_controller_precomputation.cpp
 * 
 * Implementation of precomputation methods in include/chess/game_controller.h
 * 
 */



/* INCLUDES */
#include <louischessx/game_controller.h>



/* TIME CONTROL */



/** @name  configure_search_time_paramaters
 * 
 * @brief  Based on the current time control, reconsider the search parameters.
 * @return void.
 */
void chess::game_controller::configure_search_time_paramaters ()
{
    /* Return if not in normal mode */
    if ( mode != computer_mode_t::normal ) return;

    /* Switch depending on the type of clock */
    switch ( clock_type )
    {
        /* For classical clock */
        case clock_type_t::classical:
        {
            /* Get the number of moves until the end of the time control for the computer.
             * Look one half move into the future, depending on if the player in question is next or not.
             */
            const int computer_moves_until_time_control = moves_per_control - moves_made ( next_pc != computer_pc ) % moves_per_control;
            const int opponent_moves_until_time_control = moves_per_control - moves_made ( next_pc == computer_pc ) % moves_per_control;

            /* Set the maximum response time */
            max_response_duration = computer_clock / computer_moves_until_time_control;

            /* Set the maximum thinking time. This is the sum of the response duration for both the computer and opponent. */
            max_search_duration = max_response_duration + std::max<chess_clock::duration> ( opponent_clock / opponent_moves_until_time_control, average_opponent_response_time );

            /* Break */
            break;
        }

        /* For incremental clock */
        case clock_type_t::incremental:
        {
            /* Set the maximum response time. Use up all the remaining time in the next 50 moves */
            max_response_duration = time_increase + computer_clock / 25;

            /* Set the maximum thinking time. This is the sum of the response duration for both the computer and opponent */
            max_search_duration = max_response_duration + std::max<chess_clock::duration> ( time_increase + opponent_clock / 25, average_opponent_response_time );

            /* Break */
            break;
        }
        
        /* For fixed max clock */
        case clock_type_t::fixed_max:
        {
            /* Set the maximum response time */
            max_response_duration = time_base;

            /* Set the maximum thinking time */
            max_search_duration = max_response_duration * 2;
        }
    }
}



/** @name  start_time_control
 * 
 * @brief  Consider next_pc to be starting their turn now.
 *         If it is now the computer's turn, set up the search paramaters based on their clock etc.
 * @return void
 */
void chess::game_controller::start_time_control ()
{
    /* Ignore if not in normal mode */
    if ( mode != computer_mode_t::normal ) return;

    /* Configure the search parameters */
    configure_search_time_paramaters ();

    /* Set the turn start point */
    turn_start_point = chess_clock::now ();
}



/** @name  end_time_control
 * 
 * @brief  Consider next_pc to have just finished their turn. Decrement their clock, then add any increments.
 *         If they exceeded time control limits, then return false.
 * @return Boolean, true unless a time control has been exceeded.
 */
bool chess::game_controller::end_time_control ()
{
    /* Ignore if not in normal mode */
    if ( mode != computer_mode_t::normal ) return true;

    /* If this was the first move, and the move was made by a human player */
    if ( moves_made () == 0 && next_pc != computer_pc && !opponent_is_computer )
    {
        /* If incremental clock, still apply the increment */
        if ( clock_type == clock_type_t::incremental ) opponent_clock += time_increase;

        /* Return true */
        return true;
    }

    /* Get a reference to next_pc's clock */
    chess_clock::duration& clock = ( next_pc == computer_pc ? computer_clock : opponent_clock );

    /* Get the time they took for their turn */
    const chess_clock::duration time_taken = chess_clock::now () - turn_start_point;    

    /* Get if the player has run out of time */
    const bool out_of_time = time_taken > clock + std::chrono::seconds { 1 };

    /* Reduce the clock */
    clock = std::max ( clock - time_taken, chess_clock::duration::zero () );

    /* Switch depending on the clock type to add time increments */
    switch ( clock_type )
    {
        /* For classical clock, add time if their next move will be in a new control */
        case clock_type_t::classical: if ( moves_made ( 1 ) % moves_per_control == 0 ) clock += time_base; break;

        /* For incremental clock, add time just to this player */
        case clock_type_t::incremental: clock += time_increase; break;

        /* For fixed max clock, simply set the clock to the time base */
        case clock_type_t::fixed_max: clock = time_base; break;
    }

    /* Synchronize the other player's clock, if it is not a fixed max clock, it was just their turn, an otim command has been supplied */
    if ( clock_type != clock_type_t::fixed_max && next_pc != computer_pc && opponent_sync_clock != chess_clock::duration::max () ) opponent_clock = opponent_sync_clock;

    /* If this is the opponent's move, update the rolling average opponent thinking time */
    if ( next_pc != computer_pc ) average_opponent_response_time = std::chrono::duration_cast<chess_clock::duration> ( average_opponent_response_time * 0.8 + time_taken * 0.2 );

    /* Return !out_of_time */
    return !out_of_time;
}



/* SEARCH METHODS */



/** @name  start_search
 * 
 * @brief  Start a search, pushing the search data to the bottom of active_searches.
 *         When the search finishes, the iterator to the active search will be pushed to completed_searches.
 *         search_cv will then be notified and the thread will exit.
 *         The game state will be stored, so can be safely modified after this function returns.
 * @param  cb: The chessboard state to run the search on.
 * @param  pc: The player color to search.
 * @param  opponent_move: The opponent move which lead to this state, empty move by default.
 * @param  ttable: The transposition table from previous searches. Empty by default.
 * @param  direct_response: If true, then this search is in response to an opponent move, so max_response_duration should be used instead of max_search_duration. False by default.
 * @param  output_thinking: If true, then thinking is printed. False by default.
 * @return An iterator to the search data in active_searches.
 */
chess::game_controller::search_data_it_t chess::game_controller::start_search ( const chessboard& cb, const pcolor pc, const move_t& opponent_move, chessboard::ab_ttable_t ttable, const bool direct_response, const bool output_thinking )
{
    /* Create the search data */
    active_searches.emplace_back ( cb, pc, opponent_move, std::stop_source {}, output_thinking );

    /* Get an iterator to the active search */
    search_data_it_t search_data_it = --active_searches.end ();

    /* Start an asynchronous wait for the search to finish */
    active_searches.back ().ab_result_future = std::async ( std::launch::async, [ this, search_data_it, ttable { std::move ( ttable ) }, direct_response ] () mutable
    {
        /* Wait for the search to complete */
        chessboard::ab_result_t ab_result = search_data_it->cb.alpha_beta_iterative_deepening ( search_data_it->pc, search_depths, true, std::move ( ttable ), search_data_it->end_flag.get_token(), chess_clock::now () + ( direct_response ? max_response_duration : max_search_duration ), search_data_it->cecp_thinking );

        /* Lock the mutex, add search_data_it to the list of completed searches, and unlock the mutex */
        std::unique_lock search_lock { search_mx };
        completed_searches.push_back ( search_data_it );
        search_lock.unlock ();

        /* Notify the condition variable */
        search_cv.notify_all ();

        /* Return the result */
        return ab_result;
    } );

    /* Return the iterator to the active search */
    return search_data_it;
}



/** @name  start_precomputation
 * 
 * @brief  Start a thread to precompute searches for possible opponent responses. The thread will be stored in search_controller.
 *         Setting search_end_flag to true then notifying search_cv will cancel all active searches and the controller thread will prompty finish execution.
 *         Setting known_opponent_move before doing the above will cause the controller thread to not stop the search based on known_opponent_move (but will not start it if not already started).
 *         The game state will be stored, so can be safely modified after this function returns.
 * @return void
 */
void chess::game_controller::start_precomputation ()
{
    /* Stop any active searches */
    stop_precomputation ();

    /* Empty active_searches and completed_searches */
    active_searches.clear (); completed_searches.clear (); 

    /* Set the end flag to false and the known opponent move to unknown */
    search_end_flag = false; known_opponent_move = move_t {};

    /* Start the new thread if pondering is allowed */
    if ( pondering && num_parallel_searches ) search_controller = std::thread { [ this, cb { game_cb }, pc { computer_pc }, ttable { cumulative_ttable } ] () mutable
    {
        /* Get the opponent moves */
        chessboard::ab_result_t opponent_ab_result = cb.alpha_beta_iterative_deepening ( other_color ( pc ), opponent_search_depths, false, std::move ( ttable ), std::stop_token {}, chess_clock::now () + std::chrono::milliseconds ( 750 ) );

        /* Get the ttable back */
        ttable = std::move ( opponent_ab_result.ttable );

        /* Aquire a lock on search_mx */
        std::unique_lock search_lock { search_mx };

        /* Start num_parallel_searches threads (or less if there are few opposing moves) */
        for ( int i = 0; i < num_parallel_searches && i < opponent_ab_result.moves.size (); ++i )
        {
            /* Make the move, start the search, then unmake the move */
            cb.make_move_internal ( opponent_ab_result.moves.at ( i ).first );
            start_search ( cb, pc, opponent_ab_result.moves.at ( i ).first, cb.purge_ttable ( ttable, ttable_min_bk_depth ) );
            cb.unmake_move_internal ();  
        }

        /* Loop as many times as there are opposing moves */
        for ( int i = 0; i < opponent_ab_result.moves.size (); ++i )
        {
            /* Block on the condition variable. Block until there are is another completed search, or the end flag is set */
            search_cv.wait ( search_lock, [ this, i ] () { return i < completed_searches.size () || search_end_flag; } );

            /* If the end flag is set, break */
            if ( search_end_flag ) break;

            /* Determine if there are any searches left to start */
            if ( i + num_parallel_searches < opponent_ab_result.moves.size () )
            {
                /* Start another search by making the next move, starting the search, and unmaking the move */
                cb.make_move_internal ( opponent_ab_result.moves.at ( i + num_parallel_searches ).first );
                start_search ( cb, pc, opponent_ab_result.moves.at ( i + num_parallel_searches ).first, cb.purge_ttable ( ttable, ttable_min_bk_depth ) );
                cb.unmake_move_internal ();  
            }
        }

        /* Cancel all searches, except if the search matches known_opponent_move */
        for ( search_data_t& search_data : active_searches ) if ( search_data.opponent_move != known_opponent_move ) search_data.end_flag.request_stop ();

        /* Unlock search_mx */
        search_lock.unlock ();
    } };
}



/** @name  stop_precomputation
 * 
 * @brief  Stop precomputation, if it's running.
 * @param  oppponent_move: The now known opponent move. Defaults to no move, but if provided will set known_opponent_move which has an effect described by start_precomputation.
 * @return Iterator to a search which was made based on oppponent_move, or one past the end iterator if not found.
 */
chess::game_controller::search_data_it_t chess::game_controller::stop_precomputation ( const move_t& opponent_move )
{
    /* Aquire the search lock */
    std::unique_lock search_lock ( search_mx );

    /* Set known_opponent_move and end flag */
    known_opponent_move = opponent_move; search_end_flag = true;

    /* Unlock and notify */
    search_lock.unlock (); search_cv.notify_all ();

    /* Join the controller thread */
    if ( search_controller.joinable () ) search_controller.join ();

    /* Try to find an active search based on opponent_move and return the iterator */
    return std::find_if ( active_searches.begin (), active_searches.end (), [ &opponent_move ] ( const search_data_t& search_data ) { return search_data.opponent_move == opponent_move; } );
}