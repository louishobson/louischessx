/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/bitboard.h
 * 
 * Header file for managing a chess bitboard
 * 
 * NOT IMPLEMENTED 
 * 
 * https://www.chessprogramming.org/Defended_Pawns_(Bitboards)
 * Rest of pawns
 * 
 * 
 * 
 */



/* HEADER GUARD */
#ifndef CHESSBOARD_H_INCLUDED
#define CHESSBOARD_H_INCLUDED



/* INCLUDES */
#include <array>
#include <cctype>
#include <chess/bitboard.h>
#include <iostream>
#include <string>



/* DECLARATIONS */

namespace chess
{

    /* enum pcolor
     *
     * Enum values for different colors of piece (not types)
     */
    enum class pcolor
    {
        white,
        black,
        no_piece
    };

    /* enum ptype
     *
     * Enum values for different types of piece (not colors)
     */
    enum class ptype
    {
        pawn,
        king,
        queen,
        bishop,
        knight,
        rook,
        no_piece
    };

    /** @name  bool_color
     * 
     * @brief  cast a piece color to a bool
     * @param  pc: The piece color to cast. Undefined behavior if is no_piece.
     * @return bool
     */
    constexpr bool bool_color ( pcolor pc ) noexcept;

    /** @name  other_color
     * 
     * @brief  Take a piece color and give the other color
     * @param  pc: The piece color to switch. Undefined behavior if is no_piece.
     * @return The other color of piece
     */
    constexpr pcolor other_color ( pcolor pc ) noexcept;



    /* class chessboard 
     *
     * Store and manipulate a bitboard-based chess board
     */
    class chessboard;
}



/* CHESSBOARD DEFINITION */

/* class chessboard 
 *
 * Store and manipulate a bitboard-based chess board
 */
class chess::chessboard
{
public:

    /* CONSTRUCTORS */

    /** @name  default constructor
     * 
     * @brief  Sets up an opening chess board
     */
    constexpr chessboard () = default;



    /* FORMATTING */

    /** @name  simple_format_board
     * 
     * @brief  Create a simple representation of the board.
     *         Lower-case letters mean black, upper case white.
     * @return string
     */
    std::string simple_format_board () const;



//private:

    /* TYPES */

    /* Struct for castling rights */
    struct castling_rights_t
    {
        bool can_castle = true;
        bool castle_made = false;
        bool kingside_lost = false, queenside_lost = false;
    };

    /* Check info */
    struct check_info_t
    {
        bitboard check_vectors;
        bitboard block_vectors;
    };



    /* ATTRIBUTES */

    /* Array of color bitboards */
    bitboard color_bbs [ 2 ] =
    {
        bitboard { 0x000000000000ffff },
        bitboard { 0xffff000000000000 }
    };

    /* 2D array of type and color bitboards */
    bitboard type_bbs [ 6 ] [ 2 ] =
    { 
        { bitboard { 0x000000000000ff00 }, bitboard { 0x00ff000000000000 } },
        { bitboard { 0x0000000000000010 }, bitboard { 0x0800000000000000 } },
        { bitboard { 0x0000000000000008 }, bitboard { 0x1000000000000000 } },
        { bitboard { 0x0000000000000024 }, bitboard { 0x2400000000000000 } },
        { bitboard { 0x0000000000000042 }, bitboard { 0x4200000000000000 } },
        { bitboard { 0x0000000000000081 }, bitboard { 0x8100000000000000 } }  
    };

    /* Whether white and black has castling rights.
     * Only records whether the king or rooks have been moved, not whether castling is at this time possible.
     * Indexes of the array accessed using pcolor cast to an integer.
     */
    std::array<castling_rights_t, 2> castling_rights;
    



    /* BITBOARD ACCESS */

    /** @name  get_bb
     * 
     * @brief  Gets a bitboard, by reference, based on a single piece type
     * @param  pc: One of pcolor. Undefined behavior if is no_piece.
     * @param  pt: One of ptype. Undefined behavior if is no_piece.
     * @return The bitboard for pt 
     */
    bitboard& get_bb ( pcolor pc ) noexcept { return color_bbs [ static_cast<int> ( pc ) ]; } 
    bitboard& get_bb ( pcolor pc, ptype pt ) noexcept { return type_bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pc ) ]; }
    const bitboard& get_bb ( pcolor pc ) const noexcept { return color_bbs [ static_cast<int> ( pc ) ]; }
    const bitboard& get_bb ( pcolor pc, ptype pt ) const noexcept { return type_bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pc ) ]; }

    /** @name  bb
     * 
     * @brief  Gets a bitboard, by copy, based on a single piece type
     * @param  pc: One of pcolor. Undefined behavior if is no_piece.
     * @param  pt: One of ptype. Undefined behavior if is no_piece.
     * @return The bitboard for pt
     */
    bitboard bb () const noexcept { return bb ( pcolor::white ) | bb ( pcolor::black ); }
    bitboard bb ( pcolor pc ) const noexcept { return color_bbs [ static_cast<int> ( pc ) ]; }
    bitboard bb ( pcolor pc, ptype pt ) const noexcept { return type_bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pc ) ]; }
    bitboard bb ( ptype pt ) const noexcept { return type_bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pcolor::white ) ] | type_bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pcolor::black ) ]; }
    

    /** @name  can_castle, castle_made, castle_lost
     * 
     * @brief  Gets information about castling for each color.
     *         castle_lost gives whether both kingside and castling rights have been lost.
     * @param  pc: One of pcolor. Undefined behaviour if is no_piece.
     * @return boolean
     */
    bool can_castle  ( pcolor pc ) const noexcept { return castling_rights [ static_cast<int> ( pc ) ].can_castle;  }
    bool castle_made ( pcolor pc ) const noexcept { return castling_rights [ static_cast<int> ( pc ) ].castle_made; }
    bool castle_lost ( pcolor pc ) const noexcept { return castling_rights [ static_cast<int> ( pc ) ].kingside_lost & castling_rights [ static_cast<int> ( pc ) ].queenside_lost; }




    /* PAWN CALCULATIONS */

    /** @name  pawn_interspan_bb
     * 
     * @brief  Get the interspan of the pawns
     * @return A new bitboard
     */
    [[ gnu::flatten ]]
    bitboard pawn_interspan_bb () const noexcept { return bb ( pcolor::white, ptype::pawn ).span ( compass::n ) & bb ( pcolor::black, ptype::pawn ).span ( compass::s ); }

    /** @name  pawn_safe_squares_bb
     * 
     * @brief  Get the squares, with reference to a color, such that friendly pawns defending >= opposing opposing pawns attacking
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    [[ gnu::flatten ]]
    bitboard pawn_safe_squares_bb ( pcolor pc ) const noexcept;

    /** @name  pawn_rams_bb
     * 
     * @brief  Get a color's pawns which are acting as rams to opposing pawns
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    bitboard pawn_rams_bb ( pcolor pc ) const noexcept;

    /** @name  pawn_levers_e/w_bb
     * 
     * @brief  Get a color's pawns which are participating in a east/west lever
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    bitboard pawn_levers_e_bb ( pcolor pc ) const noexcept;
    bitboard pawn_levers_w_bb ( pcolor pc ) const noexcept;

    /** @name  pawn_any_levers_bb
     * 
     * @brief  Get a color's pawns which are participating in any lever
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    bitboard pawn_any_levers_bb ( pcolor pc ) const noexcept { return pawn_levers_e_bb ( pc ) | pawn_levers_w_bb ( pc ); }

    /** @name  pawn_inner/outer/center_levers_bb
     * 
     * @brief  Get a color's pawns which are participating in inner/outer/center levers
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    bitboard pawn_inner_levers_bb  ( pcolor pc ) const noexcept;
    bitboard pawn_outer_levers_bb  ( pcolor pc ) const noexcept;
    bitboard pawn_center_levers_bb ( pcolor pc ) const noexcept;

    /** @name  pawn_doubled_in_front_bb
     * 
     * @brief  Get a color's pawns which are directly behind a friendly pawn
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    bitboard pawn_doubled_in_front_bb ( pcolor pc ) const noexcept;

    /** @name  isolanis_bb
     * 
     * @brief  Get the isolated pawns
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    bitboard isolanis_bb ( pcolor pc ) const noexcept { return bb ( pc, ptype::pawn ) & ~bb ( pc, ptype::pawn ).pawn_any_attack_fill (); }

    /** @name  half_isolanis_bb
     * 
     * @brief  Get the half isolated pawns
     * @param  pc: The color which is considered friendly
     * @return A new bitboard
     */
    bitboard half_isolanis_bb ( pcolor pc ) const noexcept;



    /* BOARD EVALUATION */

    /** @name  get_check_info
     * 
     * @brief  Get information about the check state of a color's king
     * @param  pc: The color who's king we will look at
     * @return check_info_t
     */
    [[ using gnu : flatten, noinline, hot ]]
    check_info_t get_check_info ( pcolor pc ) const;

    /** @name  is_protected
     * 
     * @brief  Returns true if the board position is protected by the player specified.
     *         There is no restriction on what piece is at the position, since any piece in the position is ignored.
     * @param  pc: The color who is defending.
     * @param  pos: The position of the cell to check the defence of.
     * @return boolean
     */
    [[ using gnu : flatten, noinline, hot ]]
    bool is_protected ( pcolor pc, unsigned pos ) const noexcept;  

    /** @name  is_in_check
     * 
     * @brief  Similar to is_protected, but considered specifically whether a king is in check
     * @param  pc: The color who's king we will look at
     * @return boolean
     */
    bool is_in_check ( pcolor pc ) const noexcept { return is_protected ( other_color ( pc ), bb ( pc, ptype::king ).trailing_zeros_nocheck () ); }

    /** @name  evaluate
     * 
     * @brief  Symmetrically evaluate the board state
     * @param  pc: The color who's move it is next.
     *         This must be the piece who's move it is next, in order to detect moves that are in check.
     * @return Integer value
     */
    [[ using gnu : flatten, noinline, hot ]]
    int evaluate ( pcolor pc ) const;



    /* BOARD LOOKUP */

    /** @name  find_color
     *
     * @brief  Determines the color of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pos: Board position
     * @return One of pcolor
     */
    pcolor find_color ( unsigned pos ) const noexcept;

    /** @name  find_type
     * 
     * @brief  Determines the type of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pos: Board position
     * @return One of ptype
     */
    ptype find_type ( unsigned pos ) const noexcept;

};



/* INCLUDE INLINE IMPLEMENTATION */
#include <chess/chessboard.hpp>



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */