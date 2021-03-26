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



/* HEADER GUARD */
#endif /* #ifndef CHESS_GAME_CONTROLLER_HPP_INCLUDED */