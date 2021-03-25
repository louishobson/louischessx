/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/game_controller.h
 * 
 * Header file for managing a chess game
 * 
 */



/* HEADER GUARD */
#ifndef GAME_CONTROLLER_H_INCLUDED
#define GAME_CONTROLLER_H_INCLUDED



/* INCLUDES */
#include <atomic>
#include <chess/chessboard.h>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <list>
#include <mutex>
#include <vector>
#include <queue>



/* DECLARATIONS */

namespace chess
{

    /* GAME CONTROLLER CLASS */

    /* class game_controller
     *
     * An instance will store and maintain a chess game based on a set of streams communicating over the UCI protocol
     */
    class game_controller;

}



/* GAME CONTROLLER DEFINITION */

/* class game_controller
 *
 * An instance will store and maintain a chess game based on a set of streams communicating over the UCI protocol
 */
class chess::game_controller
{
public:

    /** default constructor
     *
     * @brief  Construct the game controller with defailt parameters
     */
    game_controller () = default;

    

//private:

    /* TYPES */

    /* A struct to store the information for an active search */
    struct search_data_t
    {
        /* The board state the search is being run on */
        chessboard cb;

        /* The move that lead to this state (if any) */
        chessboard::move_t opponent_move;

        /* The player to move */
        pcolor pc;

        /* The end flag for the search */
        std::atomic_bool end_flag;

        /* A future to the result of the search */
        std::future<chessboard::ab_result_t> ab_result_future;
    };



    /* ATTRIBUTES */

    /* Store the main chessboard state */
    chessboard game_cb;

    /* The player who's turn it is */
    pcolor next_pc = pcolor::white;

    /* The color this program is playing as */
    const pcolor computer_pc = pcolor::white;



    /* While waiting for the opponent to make a move, the time can be used for parallel searches on different possible responses. This is known as 'pondering'.
     * When the opponent moves, any searches based on other moves the opponent could have made are cancelled.
     * 
     * If the search for their move has completed, the computer makes its computed response immediately.
     * If the search for their move is currently running, it is force cancelled after a duration max_thinking_time, if it does not complete before then.
     * If the search for their move has not started, it is started and allowed to run for max_thinking_time seconds.
     * 
     * The following details about searches can be specified:
     * 
     * A list of depths that will be searched in iterative deepening.
     * A list of depths that will be searched to determine the opponent's best moves.
     * The number of parallel searches to make. If a search finishes, further searches will be started keeping a maximum of 6 simultaneous searches.
     * The maximum time duration an search can take, at which point other opponent responses will be tried. See above about what happens if the opponent moves before or after this time us up.
     * The maximum time AFTER the opponent has moved that the computer should take searching before making a move
     */
    std::vector<int> search_depths = { 3, 4, 5, 6, 7, 8, 9, 10 };
    std::vector<int> opponent_search_depths = { 3, 4, 5, 6, 7 };
    int num_parallel_searches = 7;
    chess_clock::duration max_search_duration = std::chrono::seconds { 20 };
    chess_clock::duration max_response_duration = std::chrono::seconds { 10 };



    /* A vector of search data */
    std::list<search_data_t> active_searches;

    /* An iterator to an active search */
    typedef std::list<search_data_t>::iterator search_data_it_t;

    /* The thread that contols the parallel searches */
    std::thread search_controller;

    /* An atomic boolean acting as an end flag for the entirety of the search controller */
    std::atomic_bool controller_end_flag;

    /* An atomic move_t, which gives the known opponent response to cancel other searches in the search controller */
    std::atomic<chessboard::move_t> known_opponent_move;

    /* A mutex, condition variable, and vector of iterators of elements in active_searches to notify the controller which searches have completed */
    std::mutex search_mx;
    std::condition_variable search_cv;
    std::vector<search_data_it_t> completed_searches;



    /* METHODS */

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
    search_data_it_t start_search ( const chessboard& cb, const chessboard::move_t& mv, pcolor pc, bool direct_response = false );

    /** @name  start_precomputation
     * 
     * @brief  Start a thread to precompute searches for possible opponent responses. The thread will be stored in search_controller.
     *         Setting controller_end_flag to true then notifying search_cv will cancel all active searches and the controller thread will prompty finish execution.
     *         Setting known_opponent_move before doing the above will cause the controller thread to start the search in response to known_opponent_move, or not cancel it if already started. All other searches are cancelled.
     *         The game state will be stored, so can be safely modified after this function returns.
     * @param  pc: The color that searches are made for (based on the possible moves for other)
     * @return void
     */
    void start_precomputation ( pcolor pc );

};



/* HEADER GUARD */
#endif /* #ifndef GAME_CONTROLLER_H_INCLUDED */

