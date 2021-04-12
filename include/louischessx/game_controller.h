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
#include <louischessx/chessboard.h>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <list>
#include <mutex>
#include <vector>



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

    /* CONSTRUCTORS AND DESTRUCTORS */

    /** @name  default constructor
     *
     * @brief  Construct the game controller with defailt parameters
     */
    game_controller () = default;

    /** @name  pipe constructor
     * 
     * @brief  Construct with references to the pipes to use for input and output
     * @param  in: The input pipe to use.
     * @param  out: The output pipt to use. 
     */
    game_controller ( std::istream& in, std::ostream& out ) : chess_in { in }, chess_out { out } {}

    /** @name  destructor
     * 
     * @brief  Cancels any precomputation before destructing.
     */
    ~game_controller ();

    /** @name  reset_game
     * 
     * @brief  Reset the chess game to its initial state.
     *         Will stop any precomputation that is running.
     * @return void
     */
    void reset_game ();

    

    /* XBOARD INTERFACE */

    /** @name  xboard_loop
     * 
     * @brief  Start looping over commands recieved by chess_in, expecing an xboard-like interface.
     * @return void, when the xboard interface sends an exit command.
     */
    void xboard_loop ();



private:

    /* TYPES */

    /* An enum to store the mode the computer is currently in */
    enum class computer_mode_t
    {
        /* Normal mode */
        normal,

        /* Force mode */
        force,

        /* Analysis mode */
        analyze
    };

    /* A struct to store the information for an active search */
    struct search_data_t
    {
        /* The board state the search is being run on */
        chessboard cb;

        /* The player to move */
        pcolor pc;

        /* The move that lead to this state */
        move_t opponent_move;

        /* The end flag for the search */
        std::atomic_bool end_flag;

        /* A future to the result of the search */
        std::future<chessboard::ab_result_t> ab_result_future;
    };

    /* An enum to store the type of clock being used */
    enum class clock_type_t
    {
        /* Classical timing. 
         * Each player has an initial amount of time, which decreases during their turn.
         * After a certain number of moves, the time is increased by a given amount.
         */
        classical,

        /* Incremental timing.
         * Each player starts with initial amount of time, which decreases during their turn.
         * After each move, the amount of time is increased by a given amount.
         */
        incremental,

        /* Fixed max.
         * Each player has a given amount of time to make a move during their turn.
         * Any left over time does not carry over to their next turn.
         */
        fixed_max
    };



    /* GENERAL GAME ATTRIBUTES */

    /* Store the main chessboard state */
    chessboard game_cb;

    /* The mode of the computer */
    computer_mode_t mode = computer_mode_t::force;

    /* The player who's turn it is */
    pcolor next_pc = pcolor::white;

    /* The color the computer is playing as. Assume black initially. */
    pcolor computer_pc = pcolor::black;

    /* Whether the opponent is a computer or not */
    bool opponent_is_computer = false;

    /* The latest value a search by a computer produced */
    int latest_best_value = 0;

    /* The cumulative transposition table */
    chessboard::ab_ttable_t cumulative_ttable;

    /* The input and output streams to use */
    std::istream& chess_in = std::cin;
    std::ostream& chess_out = std::cout;

    /* The type of clock. Initially fixed_max. */
    clock_type_t clock_type = clock_type_t::fixed_max;

    /* The variables for the clock. Only time_base is used if the clock type is fixed_max. */
    chess_clock::duration time_base = std::chrono::seconds { 15 };
    chess_clock::duration time_increase;
    chess_clock::duration computer_clock, opponent_clock;
    int moves_per_control;

    /* Sync clock for the opponent */
    chess_clock::duration opponent_sync_clock = chess_clock::duration::max ();



    /* SEARCH PARAMETER ATTRIBUTES */

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
     * The maximum time AFTER the opponent has moved that the computer should take searching before making a move.
     * The minimum bk_depth for an entry in the cumulative ttable entry to be considered worth keeping.
     * The value of latest_best_value for a draw offer to be considered a good idea.
     */
    std::vector<int> search_depths = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    std::vector<int> opponent_search_depths = { 3, 4, 5, 6 };
    int num_parallel_searches = 4;
    chess_clock::duration max_search_duration = std::chrono::seconds { 30 };
    chess_clock::duration max_response_duration = std::chrono::seconds { 15 };
    int ttable_min_bk_depth = 4;
    int draw_offer_acceptance_value = -100;



    /* ACTIVE SEARCH ATTRIBUTES */

    /* The time at which the current color's turn began */
    chess_clock::time_point turn_start_point;

    /* A list of search data */
    std::list<search_data_t> active_searches;

    /* An iterator to an active search */
    typedef std::list<search_data_t>::iterator search_data_it_t;

    /* The thread that contols the parallel searches */
    std::thread search_controller;

    /* A mutex and condition variable for protecting shared resources of the search */
    std::mutex search_mx;
    std::condition_variable search_cv;

    /* The following three variables must be protected by search_mx when accessing */

    /* A vector of iterators of elements in active_searches to notify the controller which searches have completed */
    std::vector<search_data_it_t> completed_searches;

    /* A boolean acting as an end flag for the entirety of the search controller */
    bool search_end_flag;

    /* A move_t, which gives the known opponent response to cancel other searches in the search controller */
    move_t known_opponent_move; 



    /* COMMAND HANDLING METHODS */

    /** @name  handle_command
     * 
     * @brief  Take a command and fully handle it before returning.
     *         Some commands are considered fatal, since they are both significant and unhandled, which will cause an exception.
     *         Some commands may start a new thread and return immediately, this being considered having handled the command.
     * @param  cmd: The command to handle, with its arguments separated by spaces (with or without a newline).
     * @return True if the command was handled, false if it was unrecognised (thus ignored).
     */
    bool handle_command ( const std::string& cmd );

    /** @name  safe_stoi
     * 
     * @brief  Call std::stoi, but rethrow an exception as a chess_input_error.
     * @param  str: The string to convert
     * @return The converted integer.
     */
    int safe_stoi ( const std::string& str ) const;

    /** @name  make_and_output_move
     * 
     * @brief  Takes an ab_result and performs, then outputs the best move, if there is one, as well as a result if the game has ended.
     *         Also starts precomputation if not in force mode.
     * @param  ab_result: The result of the search on this state.
     * @return void
     */
    void make_and_output_move ( chessboard::ab_result_t& ab_result );



    /* TIME CONTROL */

    /** @name  half_moves_made
     * 
     * @brief  Get the number of half moves made since the beginning of the game.
     * @param  future: The number of half moves in the future that this figure should be given for.
     * @return integer
     */
    int half_moves_made ( int future = 0 ) const noexcept { return game_cb.game_state_history.size () - 1 + future; }

    /** @name  moves_made
     * 
     * @brief  Get the number of full moves made since the beginning of the game.
     * @param  future: The number of HALF MOVES in the future that this figure should be given for.
     * @return integer
     */
    int moves_made ( int future = 0 ) const noexcept { return half_moves_made ( future ) / 2; }

    /** @name  configure_search_time_paramaters
     * 
     * @brief  Based on the current time control, reconsider the search parameters.
     * @return void.
     */
    void configure_search_time_paramaters ();

    /** @name  start_time_control
     * 
     * @brief  Consider next_pc to be starting their turn now.
     *         If it is now the computer's turn, set up the search paramaters based on their clock etc.
     * @return void
     */
    void start_time_control ();

    /** @name  end_time_control
     * 
     * @brief  Consider next_pc to have just finished their turn. Decrement their clock, then add any increments.
     *         If they exceeded time control limits, then return false.
     * @return Boolean, true unless a time control has been exceeded.
     */
    bool end_time_control ();



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
     * @return An iterator to the search data in active_searches.
     */
    search_data_it_t start_search ( const chessboard& cb, pcolor pc, const move_t& opponent_move = move_t {}, chessboard::ab_ttable_t ttable = chessboard::ab_ttable_t {}, bool direct_response = false );

    /** @name  start_precomputation
     * 
     * @brief  Start a thread to precompute searches for possible opponent responses. The thread will be stored in search_controller.
     *         Setting search_end_flag to true then notifying search_cv will cancel all active searches and the controller thread will prompty finish execution.
     *         Setting known_opponent_move before doing the above will cause the controller thread to not stop the search based on known_opponent_move (but will not start it if not already started).
     *         The game state will be stored, so can be safely modified after this function returns.
     * @return void
     */
    void start_precomputation ();

    /** @name  stop_precomputation
     * 
     * @brief  Stop precomputation, if it's running.
     * @param  oppponent_move: The now known opponent move. Defaults to no move, but if provided will set known_opponent_move which has an effect described by start_precomputation.
     * @return Iterator to a search which was made based on opponent_move, or one past the end iterator if not found.
     */
    search_data_it_t stop_precomputation ( const move_t& opponent_move = {} );

};



/* INCLUDE INLINE IMPLEMENTATION */
#include <louischessx/game_controller.hpp>



/* HEADER GUARD */
#endif /* #ifndef GAME_CONTROLLER_H_INCLUDED */