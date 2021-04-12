/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * include/chess/game_contoller.hpp
 *
 * Inline implementation of general functions in include/chess/game_controller.h
 *
 */



/* HEADER GUARD */
#ifndef CHESS_GAME_CONTROLLER_HPP_INCLUDED
#define CHESS_GAME_CONTROLLER_HPP_INCLUDED



/* INCLUDES */
#include <chess/game_controller.h>



/* CONSTRUCTORS AND DESTRUCTORS */



/** @name  destructor
 * 
 * @brief  Cancels any precomputation before destructing.
 */
inline chess::game_controller::~game_controller ()
{
    /* Stop any precomputation */
    stop_precomputation ();

    /* Empty active_searches and completed_searches */
    active_searches.clear (); completed_searches.clear ();
}



/** @name  reset_game
 * 
 * @brief  Reset the chess game to its initial state.
 *         Will stop any precomputation that is running.
 * @return void
 */
inline void chess::game_controller::reset_game ()
{
    /* Stop any precomputation */
    stop_precomputation ();

    /* Reset active and complete searches */
    active_searches.clear (); completed_searches.clear ();

    /* Reset the game state and color to move */
    game_cb = {}; next_pc = pcolor::white;
}



/* XBOARD INTERFACE */



/** @name  xboard_loop
 * 
 * @brief  Start looping over commands recieved by chess_in, expecing an xboard-like interface.
 * @return void, when the xboard interface sends an exit command.
 */
inline void chess::game_controller::xboard_loop ()
{
    /* Store incomming commands */
    std::string cmd;

    /* Loop until quit command */
    do
    {
        /* Read a line from chess_in */
        std::getline ( chess_in, cmd );
        
        /* Remove the endline, if present */
        if ( cmd.back () == '\n' ) cmd.pop_back ();

        /* Handle the command */
        handle_command ( cmd );

        /* Break on a quit command */
    } while ( !cmd.starts_with ( "quit" ) );
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_GAME_CONTROLLER_HPP_INCLUDED */