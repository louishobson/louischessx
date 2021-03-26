/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/game_controller.cpp
 * 
 * Implementation of include/chess/game_controller.h
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
 * @param  mv: The opponent move that lead to that state.
 * @param  pc: The player color to search.
 * @param  direct_response: If true, then this search is in response to an opponent move, so max_response_duration should be used instead of max_search_duration. False by default.
 * @return An iterator to the search data in active_searches.
 */
chess::game_controller::search_data_it_t chess::game_controller::start_search ( const chessboard& cb, const chessboard::move_t& mv, const pcolor pc, const bool direct_response )
{
    /* Create the search data */
    active_searches.emplace_back ( cb, mv, pc, false );

    /* Get an iterator to the active search */
    search_data_it_t search_data_it = --active_searches.end ();

    /* Start an asynchronous wait for the search to finish */
    active_searches.back ().ab_result_future = std::async ( std::launch::async, [ this, search_data_it, direct_response ] () mutable
    {
        /* Wait for the search to complete */
        chessboard::ab_result_t ab_result = search_data_it->cb.alpha_beta_iterative_deepening ( search_data_it->pc, search_depths, true, search_data_it->end_flag, chess_clock::now () + ( direct_response ? max_response_duration : max_search_duration ) );

        /* Lock the mutex, add search_data_it to the list of completed searches, and unlock the mutex */
        std::unique_lock lock { search_mx };
        completed_searches.push_back ( search_data_it );
        lock.unlock ();

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
 *         Setting controller_end_flag to true then notifying search_cv will cancel all active searches and the controller thread will prompty finish execution.
 *         Setting known_opponent_move before doing the above will cause the controller thread to start the search in response to known_opponent_move, or not cancel it if already started. All other searches are cancelled.
 *         The game state will be stored, so can be safely modified after this function returns.
 * @param  pc: The color that searches are made for (based on the possible moves for other)
 * @return void
 */
void chess::game_controller::start_precomputation ( const pcolor pc )
{
    /* Stop any active searches */
    for ( search_data_t& search_data : active_searches ) { search_data.end_flag = true; search_data.ab_result_future.wait (); }

    /* Empty active_searches and completed_searches */
    active_searches.clear (); completed_searches.clear (); 

    /* Set the end flag to false and the known opponent move to unknown */
    controller_end_flag = false; known_opponent_move = chessboard::move_t {};

    /* Start the new thread */
    search_controller = std::thread { [ this, cb { game_cb }, pc ] () mutable
    {
        /* Get the opponent moves */
        std::atomic_bool opponent_end_flag = false;
        chessboard::ab_result_t opponent_ab_result = cb.alpha_beta_iterative_deepening ( other_color ( pc ), opponent_search_depths, false, opponent_end_flag );

        /* Aquire a lock on search_mx */
        std::unique_lock lock { search_mx };

        /* Start num_parallel_searches threads (or less if there are few opposing moves) */
        for ( int i = 0; i < num_parallel_searches && i < opponent_ab_result.moves.size (); ++i )
        {
            /* Make the move, start the search, then unmake the move */
            cb.make_move_internal ( opponent_ab_result.moves.at ( i ).first );
            start_search ( cb, opponent_ab_result.moves.at ( i ).first, pc );
            cb.unmake_move_internal ();  
        }

        /* Loop as many times as there are opposing moves */
        for ( int i = 0; i < opponent_ab_result.moves.size (); ++i )
        {
            /* Block on the condition variable. Block until there are is another completed search, or the end flag is set */
            search_cv.wait ( lock, [ this, i ] () { return i < completed_searches.size () || controller_end_flag; } );
            
            std::cout << cb.fide_serialize_move ( completed_searches.back ()->opponent_move ) << "\n";

            /* If the end flag is set, break */
            if ( controller_end_flag ) break;

            /* Determine if there are any searches left to start */
            if ( i + num_parallel_searches < opponent_ab_result.moves.size () )
            {
                /* Start another search by making the next move, starting the search, and unmaking the move */
                cb.make_move_internal ( opponent_ab_result.moves.at ( i + num_parallel_searches ).first );
                start_search ( cb, opponent_ab_result.moves.at ( i + num_parallel_searches ).first, pc );
                cb.unmake_move_internal ();  
            }
        }

        /* Unlock search_mx */
        lock.unlock ();

        /* Boolean for if there is a known move and it has not been started. Set initially to whether there is a known opposing move or not. */
        bool known_move_search_not_started = ( known_opponent_move.load ().pt != ptype::no_piece ); 

        /* If the end flag is set, cancel all searches, except if the search is for the known opposing move */
        if ( controller_end_flag ) for ( search_data_t& search_data : active_searches ) if ( search_data.opponent_move != known_opponent_move ) 
            { search_data.end_flag = true; search_data.ab_result_future.wait (); } else known_move_search_not_started = false;

        /* If the known move search has not been started, start it now */
        if ( known_move_search_not_started )
        {
            /* Make the known opponent move, start the search, and unmake the move */
            cb.make_move_internal ( known_opponent_move );
            start_search ( cb, known_opponent_move, pc, true );
            cb.unmake_move_internal ();  
        }
    } };
}