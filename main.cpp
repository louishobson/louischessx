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
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <louischessx/chess.h>



/* PROGRAM OPTIONS NAMESPACE */
namespace po = boost::program_options;



/** @name  main
 * 
 * @brief  Main function
 * @param  argc: The number of command line parameters
 * @param  argv: The command line parameters.
 * @return 0, unless an error occured.
 */
int main ( const int argc, const char ** argv )
{
    /* Create a complete options description for the executable */
    po::options_description options_desc;
    options_desc.add_options ()

        /* Help option */
        ( "help,h", "produce help message" )

        /* Debug options */
        ( "debug,d", po::value<std::string> (), "path to write debug info to" )

        /* Threading options */
        ( "threads,t", po::value<int> ()->default_value ( 4 ), "the number of threads while pondering" );

    /* Create a variables map and extract the command line arguments from argc and argv */
    po::variables_map variables_map;
    po::store ( po::parse_command_line ( argc, argv, options_desc ), variables_map );
    po::notify ( variables_map );

    /* If the help option was given, output the help and return */
    if ( variables_map.count ( "help" ) )
    {
        /* Output the help */
        std::cout << options_desc << std::endl;

        /* Return 0 */
        return 0;
    }



    /* Create the game controller */
    chess::game_controller game_controller;

    /* If debug is specified, try to open the log file */
    if ( variables_map.count ( "debug" ) ) game_controller.open_log_file ( variables_map.at ( "debug" ).as<std::string> () );

    /* Set the number of parallel searches */
    game_controller.set_parallel_searches ( variables_map.at ( "threads" ).as<int> () );

    /* Start the xboard communication loop */
    game_controller.xboard_loop ();



    /* Loop finished, so return 0 */
    return 0;
}