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
#include <chess/game_controller.h>



/* SEARCH METHODS */



/** @name  start_search
 * 
 * @brief  Start a search, pushing the search data to the bottom of active_searches.
 *         When the search finishes, the iterator to the active search will be pushed to completed_searched (protected by search_mx).
 *         search_cv will then be notified and the thread will exit.
 *         The game state will be stored, so can be safely modified after this function returns.
 * @param  cb: The chessboard state to run the search on.
 * @param  pc: The player color to search.
 * @param  opponent_move: The opponent move which lead to this state, empty move by default.
 * @param  ttable: The transposition table from previous searches. Empty by default.
 * @param  direct_response: If true, then this search is in response to an opponent move, so max_response_duration should be used instead of max_search_duration. False by default.
 * @return An iterator to the search data in active_searches.
 */
chess::game_controller::search_data_it_t chess::game_controller::start_search ( const chessboard& cb, const pcolor pc, const move_t& opponent_move, chessboard::ab_ttable_t ttable, const bool direct_response )
{
    /* Create the search data */
    active_searches.emplace_back ( cb, pc, opponent_move, false );

    /* Get an iterator to the active search */
    search_data_it_t search_data_it = --active_searches.end ();

    /* Start an asynchronous wait for the search to finish */
    active_searches.back ().ab_result_future = std::async ( std::launch::async, [ this, search_data_it, ttable { std::move ( ttable ) }, direct_response ] () mutable
    {
        /* Wait for the search to complete */
        chessboard::ab_result_t ab_result = search_data_it->cb.alpha_beta_iterative_deepening ( search_data_it->pc, search_depths, true, std::move ( ttable ), search_data_it->end_flag, chess_clock::now () + ( direct_response ? max_response_duration : max_search_duration ) );

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
 *         Setting known_opponent_move before doing the above will cause the controller thread to start the search in response to known_opponent_move, or not cancel it if already started. All other searches are cancelled.
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

    /* Start the new thread */
    search_controller = std::thread { [ this, cb { game_cb }, pc { computer_pc }, ttable { cumulative_ttable } ] () mutable
    {
        /* Get the opponent moves */
        chessboard::ab_result_t opponent_ab_result = cb.alpha_beta_iterative_deepening ( other_color ( pc ), opponent_search_depths, false, std::move ( ttable ) );

        /* Get the ttable back */
        ttable = std::move ( opponent_ab_result.ttable );

        /* Aquire a lock on search_mx */
        std::unique_lock search_lock { search_mx };

        /* Start num_parallel_searches threads (or less if there are few opposing moves) */
        for ( int i = 0; i < num_parallel_searches && i < opponent_ab_result.moves.size (); ++i )
        {
            /* Make the move, start the search, then unmake the move */
            cb.make_move_internal ( opponent_ab_result.moves.at ( i ).first );
            start_search ( cb, pc, opponent_ab_result.moves.at ( i ).first, cb.purge_ttable ( ttable ) );
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
                start_search ( cb, pc, opponent_ab_result.moves.at ( i + num_parallel_searches ).first, cb.purge_ttable ( ttable ) );
                cb.unmake_move_internal ();  
            }
        }

        /* Cancel all searches, except if the search matches known_opponent_move */
        for ( search_data_t& search_data : active_searches ) if ( search_data.opponent_move != known_opponent_move ) search_data.end_flag = true;

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