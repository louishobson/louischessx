/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/bitboard.cpp
 * 
 * implementation of include/chess/bitboard.h
 * 
 */



/* INCLUDES */
#include <chess/bitboard.h>



/* BITBOARD MASK DEFINITIONS */
const chess::bitboard chess::masks::light_squares { 0x55AA55AA55AA55AA };
const chess::bitboard chess::masks::dark_squares  { 0xAA55AA55AA55AA55 };