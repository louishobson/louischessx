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
#include <louischessx/game_controller.h>



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



/* GENERAL OPTIONS */



/** @name  open_log_file
 * 
 * @brief  Tries to open a file to log to, and enabled logging.
 * @param  path: The path of the logfile to open.
 * @return void.
 */
inline void chess::game_controller::open_log_file ( const std::string& path )
{
    /* Try to open the file for writing */
    chess_log.open ( path, std::ios::out );

    /* If failed, throw */
    if ( !chess_log ) chess::chess_input_error { "Failed to open log file." };

    /* Enable logging */
    output_log = true;
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
        /* Get the next command */
        cmd = read_chess_in ();

        /* Handle the command */
        handle_command ( cmd );

        /* Break on a quit command */
    } while ( !cmd.starts_with ( "quit" ) );
}



/* INPUT AND OUTPUT */



/** @name  read_chess_in
 * 
 * @brief  Returns the next line availible from chess_in.
 *         Also logs to chess_log, if enabled by output_log.
 * @return String, with the newline stripped.
 */
inline std::string chess::game_controller::read_chess_in ()
{
    /* Get a command */
    std::string cmd; std::getline ( chess_in, cmd );
        
    /* Remove the endline, if present */
    if ( cmd.back () == '\n' ) cmd.pop_back ();

    /* Output log if required */
    if ( output_log ) chess_log << ">  " << cmd << std::endl;

    /* Return the command */
    return cmd;
}



/** @name  write_chess_out
 * 
 * @brief  Write a command or response to chess_out.
 *         A newline will be added and the stream will be flushed.
 *         Also logs to chess_log, if enabled by output_log.
 * @param  outputs...: Parameters of printable types to send to chess_out and maybe chess_log.
 * @return void.
 */
template<class... Ts>
inline void chess::game_controller::write_chess_out ( const Ts&... outputs )
{
    /* Send to chess_out */
    ( chess_out << ... << outputs ) << std::endl;

    /* Output log if required */
    if ( output_log ) ( chess_log << " < " << ... << outputs ) << std::endl;
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_GAME_CONTROLLER_HPP_INCLUDED */