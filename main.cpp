/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * main.cpp
 * 
 * Main entry file for the xboard interface
 * 
 */



/* INCLUDES */
#include <louischessx/chess.h>
#include <iostream>



/** @name  main
 * 
 * @brief  Main function
 * @param  argc: The number of command line parameters
 * @param  argv: The command line parameters.
 * @return 0, unless an error occured.
 */
int main ( const int argc, const char ** argv )
{
    /* Create the engine */
    chess::game_controller game_controller;

    /* Start the xboard communication */
    game_controller.xboard_loop ();

    /* Return 0 */
    return 0;
}