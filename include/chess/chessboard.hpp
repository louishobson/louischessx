/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * include/chess/chessboard.hpp
 *
 * Inline implementation of include/chess/chessboard.h
 *
 */



/* HEADER GUARD */
#ifndef CHESS_CHESSBOARD_HPP_INCLUDED
#define CHESS_CHESSBOARD_HPP_INCLUDED



/* INCLUDES */
#include <chess/chessboard.h>



/* PIECE ENUMS */



/** @name  bool_color
 * 
 * @brief  cast a piece color to a bool
 * @param  pc: The piece color to cast. Undefined behavior if is no_piece.
 * @return bool
 */
inline constexpr bool chess::bool_color ( pcolor pc ) noexcept { return static_cast<bool> ( pc ); }

/** @name  other_color
 * 
 * @brief  Take a piece color and give the other color
 * @param  pc: The piece color to switch. Undefined behavior if is no_piece.
 * @return The other color of piece
 */
inline constexpr chess::pcolor chess::other_color ( pcolor pc ) noexcept { return static_cast<pcolor> ( !static_cast<bool> ( pc ) ); }



/* PAWN CALCULATIONS */



/** @name  pawn_safe_squares_bb
 * 
 * @brief  Get the squares, with reference to a color, such that friendly pawns defending >= opposing opposing pawns attacking
 * @param  pc: The color which is considered defeanding
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_safe_squares_bb ( pcolor pc ) const noexcept
{
    /* Get all attcks, white and black, east and west */
    bitboard w_east_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne );
    bitboard w_west_attack = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw );
    bitboard b_east_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se );
    bitboard b_west_attack = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw );

    /* Switch depending on pc.
     * Any cell defeanded twice is safe.
     * Any cells opponent is not attacking are safe.
     * Any cells that friendly is attacking once, and opponent is not double attacking are safe.
     */
    if ( pc == pcolor::white )
        return ( w_east_attack & w_west_attack ) | ~( b_east_attack | b_west_attack ) | ( ( w_east_attack ^ w_west_attack ) & ~( b_east_attack & b_west_attack ) );
    else
        return ( b_east_attack & b_west_attack ) | ~( w_east_attack | w_west_attack ) | ( ( b_east_attack ^ b_west_attack ) & ~( w_east_attack & w_west_attack ) );
}

/** @name  pawn_rams_bb
 * 
 * @brief  Get a color's pawns which are acting as rams to opposing pawns
 * @param  pc: The color which is considered to be blocking
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_rams_bb ( pcolor pc ) const noexcept
{
    /* Switch depending on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::s );
    else
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::n );
}

/** @name  pawn_levers_e/w_bb
 * 
 * @brief  Get a color's pawns which are participating in a east/west lever
 * @param  pc: The color which is considered friendly
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_levers_e_bb ( pcolor pc ) const noexcept
{
    /* Switch depending on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::sw );
    else
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::nw );
}
inline chess::bitboard chess::chessboard::pawn_levers_w_bb ( pcolor pc ) const noexcept
{
    /* Switch depending on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::se );
    else
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::ne );
}

/** @name  pawn_inner/outer/center_levers_bb
 * 
 * @brief  Get a color's pawns which are participating in inner/outer/center levers
 * @param  pc: The color which is considered friendly
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::pawn_inner_levers_bb ( pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard abc_files { bitboard::masks::file_a | bitboard::masks::file_b | bitboard::masks::file_c }; 
    constexpr bitboard fgh_files { bitboard::masks::file_f | bitboard::masks::file_g | bitboard::masks::file_h }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & abc_files ) | ( pawn_levers_w_bb ( pc ) & fgh_files );
}
inline chess::bitboard chess::chessboard::pawn_outer_levers_bb ( pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard bcd_files { bitboard::masks::file_b | bitboard::masks::file_c | bitboard::masks::file_d }; 
    constexpr bitboard efg_files { bitboard::masks::file_e | bitboard::masks::file_f | bitboard::masks::file_g }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & efg_files ) | ( pawn_levers_w_bb ( pc ) & bcd_files );
}
inline chess::bitboard chess::chessboard::pawn_center_levers_bb ( pcolor pc ) const noexcept
{
    /* Masks for detecting position of levers */
    constexpr bitboard d_file { bitboard::masks::file_d }; 
    constexpr bitboard e_file { bitboard::masks::file_g }; 

    /* Return the levers */
    return ( pawn_levers_e_bb ( pc ) & d_file ) | ( pawn_levers_w_bb ( pc ) & e_file );
}

/** @name  pawn_doubled_in_front_bb
 * 
 * @brief  Get a color's pawns which are directly behind a friendly pawn
 * @param  pc: The color which is considered friendly
 * @return A bitboard
 */
inline chess::bitboard chess::chessboard::pawn_doubled_in_front_bb ( pcolor pc ) const noexcept
{
    /* Switch on pc */
    if ( pc == pcolor::white )
        return bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).fill ( compass::s );
    else 
        return bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).fill ( compass::n );
}

/** @name  half_isolanis_bb
 * 
 * @brief  Get the half isolated pawns
 * @param  pc: The color which is considered friendly
 * @return A new bitboard
 */
inline chess::bitboard chess::chessboard::half_isolanis_bb ( pcolor pc ) const noexcept
{
    /* Return the half isolanis */
    bitboard pawns = bb ( pc, ptype::pawn );
    return ( pawns & ~pawns.pawn_attack_fill_e () ) ^ ( pawns & ~pawns.pawn_attack_fill_w () );    
}



/* BOARD EVALUATION */



/** @name  get_check_info
 * 
 * @brief  Get information about the check state of a color's king
 * @param  pc: The color who's king we will look at
 * @return check_info_t
 */
inline chess::chessboard::check_info_t chess::chessboard::get_check_info ( pcolor pc ) const noexcept
{
    /* SETUP */

    /* The output check info */
    check_info_t check_info;

    /* Get the friendly and opposing pieces */
    bitboard friendly = bb ( pc );
    bitboard opposing = bb ( other_color ( pc ) );

    /* Get the king and position of the colored king */
    bitboard king = friendly & bb ( ptype::king );
    unsigned king_pos = king.trailing_zeros_nocheck ();

    /* Get the positions of the opposing straight and diagonal pieces */
    bitboard op_straight = opposing & ( bb ( ptype::queen ) | bb ( ptype::rook ) );
    bitboard op_diagonal = opposing & ( bb ( ptype::queen ) | bb ( ptype::bishop ) );

    /* Get the primary propagator.
     * Primary propagator of not opposing means that spans will overlook white pieces.
     * The secondary propagator will be universe.
     */
    bitboard pp = ~opposing;
    bitboard sp = ~bitboard {};



    /* SLIDING PIECES */

    /* Get the start of the compasses */
    straight_compass straight_dir = straight_compass_start ();
    diagonal_compass diagonal_dir = diagonal_compass_start ();

    /* Iterate through the straight compass to see if those sliding pieces could be attacking */
    if ( bitboard::straight_attack_lookup ( king_pos ) & op_straight )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( straight_dir ), king_pos ) & op_straight )
        {
            /* Span the king in the current direction */
            bitboard king_span = king.rook_attack ( straight_dir, pp, sp );

            /* Increment compass */
            straight_dir = compass_next ( straight_dir );

            /* Get the checking and blocking pieces */
            bitboard checking = king_span & op_straight;
            bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.checking |= checking.only_if  ( blocking.is_empty () );
            check_info.vectors  |= king_span.only_if ( blocking.is_empty () );
            check_info.blocking |= blocking.only_if  ( blocking.is_singleton () );
        }
    }

    /* Iterate through the diagonal compass to see if those sliding pieces could be attacking */
    if ( bitboard::diagonal_attack_lookup ( king_pos ) & op_diagonal )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( diagonal_dir ), king_pos ) & op_diagonal )
        {
            /* Span the king in the current direction */
            bitboard king_span = king.bishop_attack ( diagonal_dir, pp, sp );

            /* Increment compass */
            diagonal_dir = compass_next ( diagonal_dir );

            /* Get the checking and blocking pieces */
            bitboard checking = king_span & op_diagonal;
            bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.checking |= checking.only_if  ( blocking.is_empty () );
            check_info.vectors  |= king_span.only_if ( blocking.is_empty () );
            check_info.blocking |= blocking.only_if  ( blocking.is_singleton () );
        }
    }



    /* KNIGHTS AND PAWNS */

    /* Simply add checking knights */
    check_info.checking |= bitboard::knight_attack_lookup ( king_pos ) & opposing & bb ( ptype::knight );
    check_info.vectors  |= check_info.checking;

    /* Switch depending on pc and add checking pawns */
    check_info.checking |= ( pc == pcolor::white ? king.pawn_any_attack_n () : king.pawn_any_attack_s () ) & opposing & bb ( ptype::pawn );
    check_info.vectors  |= check_info.checking;



    /* Return the info */
    return check_info;    
}



/** @name  evaluate
 * 
 * @brief  Symmetrically evaluate the board state
 * @param  pc: The color who's move it is next. This is soley used to check that the previous player did not leave themselves in check.
 * @return Decimal value
 */
inline float chess::chessboard::evaluate ( pcolor pc ) const noexcept
{
    /* CONSTANTS */

    /* Evaluation constants */
    constexpr float QUEEN  { 9.0 };
    constexpr float ROOK   { 5.0 };
    constexpr float BISHOP { 3.0 };
    constexpr float KNIGHT { 3.0 };
    constexpr float PAWN   { 1.0 };

    constexpr float MOVABILITY { 0.1 };

    constexpr float DOUBLED_PAWNS { -0.5 };
    constexpr float DOUBLE_BISHOP {  0.5 };



    /* SETUP */

    /* Define boards and variables for further information collection */
    struct color_vars_t
    {
        /* The pieces */
        bitboard pieces, king, queen, rooks, bishops, knights, pawns;

        /* Unions of straight and diagonal sliding pieces (queen and rook; queen and bishop) */
        bitboard straight_pieces, diagonal_pieces;

        /* Secondary propagator set (primary is the same for both colors) */
        bitboard sp;

        /* The union of all attack sets */
        bitboard attack_union;
        
        /* The number of availible moves */
        unsigned movability { 0 };

        /* Whether this color is in check */
        bool in_check { false };
    } white_vars, black_vars;

    /* Set the pieces */
    white_vars.pieces  = bb ( pcolor::white );                black_vars.pieces  = bb ( pcolor::black );
    white_vars.king    = bb ( pcolor::white, ptype::king );   black_vars.king    = bb ( pcolor::black, ptype::king );
    white_vars.queen   = bb ( pcolor::white, ptype::queen );  black_vars.queen   = bb ( pcolor::black, ptype::queen );
    white_vars.rooks   = bb ( pcolor::white, ptype::rook );   black_vars.rooks   = bb ( pcolor::black, ptype::rook );
    white_vars.bishops = bb ( pcolor::white, ptype::bishop ); black_vars.bishops = bb ( pcolor::black, ptype::bishop );
    white_vars.knights = bb ( pcolor::white, ptype::knight ); black_vars.knights = bb ( pcolor::black, ptype::knight );
    white_vars.pawns   = bb ( pcolor::white, ptype::pawn );   black_vars.pawns   = bb ( pcolor::black, ptype::pawn );

    /* Set straight and diagonal pieces */
    white_vars.straight_pieces = white_vars.queen | white_vars.rooks;
    white_vars.diagonal_pieces = white_vars.queen | white_vars.bishops;
    black_vars.straight_pieces = black_vars.queen | black_vars.rooks;
    black_vars.diagonal_pieces = black_vars.queen | black_vars.bishops;

    /* Set the primary and secondary propagators */
    bitboard pp = ~( white_vars.pieces | black_vars.pieces );
    white_vars.sp = ~white_vars.pieces;
    black_vars.sp = ~black_vars.pieces;



    /* CHECK INFO */

    /* Get the check info */
    check_info_t check_info = get_check_info ( pc );



    /* SLIDING PIECES */

    /* Get the start of the compasses */
    straight_compass straight_dir = straight_compass_start ();
    diagonal_compass diagonal_dir = diagonal_compass_start ();

    /* Iterate through the compass to get all queen, rook and bishop attacks */
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Apply all shifts */
        bitboard white_straight_attacks = white_vars.straight_pieces.rook_attack   ( straight_dir, pp, white_vars.sp );
        bitboard white_diagonal_attacks = white_vars.diagonal_pieces.bishop_attack ( diagonal_dir, pp, white_vars.sp );
        bitboard black_straight_attacks = black_vars.straight_pieces.rook_attack   ( straight_dir, pp, black_vars.sp );
        bitboard black_diagonal_attacks = black_vars.diagonal_pieces.bishop_attack ( diagonal_dir, pp, black_vars.sp );

        /* Sum movability */
        white_vars.movability += white_straight_attacks.popcount () + white_diagonal_attacks.popcount ();
        black_vars.movability += black_straight_attacks.popcount () + black_diagonal_attacks.popcount ();

        /* Union attacks */
        white_vars.attack_union |= white_straight_attacks | white_diagonal_attacks;
        black_vars.attack_union |= black_straight_attacks | black_diagonal_attacks;

        /* Increment compasses */
        straight_dir = compass_next ( straight_dir );
        diagonal_dir = compass_next ( diagonal_dir );
    }

    /* Calculate if anyone is in check */
    white_vars.in_check |= static_cast<bool> ( black_vars.attack_union & white_vars.king );
    black_vars.in_check |= static_cast<bool> ( white_vars.attack_union & black_vars.king );



    /* KNIGHTS */

    /* Iterate through white knights to get attacks */
    while ( white_vars.knights )
    {
        /* Get the position of the next knight then use the position to lookup the attacks */
        unsigned pos = white_vars.knights.trailing_zeros_nocheck ();
        bitboard knight_attacks = bitboard::knight_attack_lookup ( pos ) & white_vars.sp;   

        /* Sum movability and union attacks */
        white_vars.movability += knight_attacks.popcount ();
        white_vars.attack_union |= knight_attacks;

        /* Unset this bit */
        white_vars.knights.reset ( pos );
    } white_vars.knights = bb ( pcolor::white, ptype::knight );

    /* Iterate through black knights to get attacks */
    while ( black_vars.knights )
    {
        /* Get the position of the next knight then use the position to lookup the attacks */
        unsigned pos = black_vars.knights.trailing_zeros_nocheck ();
        bitboard knight_attacks = bitboard::knight_attack_lookup ( pos ) & black_vars.sp;   

        /* Sum movability and union attacks */
        black_vars.movability += knight_attacks.popcount ();
        black_vars.attack_union |= knight_attacks;

        /* Unset this bit */
        black_vars.knights.reset ( pos );
    } black_vars.knights = bb ( pcolor::black, ptype::knight );

    /* Calculate if anyone is in check */
    white_vars.in_check |= static_cast<bool> ( black_vars.attack_union & white_vars.king );
    black_vars.in_check |= static_cast<bool> ( white_vars.attack_union & black_vars.king );



    /* PAWNS AND KING */

    /* Calculations for pawn pushes */
    bitboard white_pawn_pushes = white_vars.pawns.pawn_push_n ( pp );
    bitboard black_pawn_pushes = black_vars.pawns.pawn_push_s ( pp );

    /* Calculations for pawn attacks */
    bitboard white_pawn_attacks_e = white_vars.pawns.pawn_attack ( diagonal_compass::ne, black_vars.pieces );
    bitboard white_pawn_attacks_w = white_vars.pawns.pawn_attack ( diagonal_compass::nw, black_vars.pieces );
    bitboard black_pawn_attacks_e = black_vars.pawns.pawn_attack ( diagonal_compass::se, white_vars.pieces );
    bitboard black_pawn_attacks_w = black_vars.pawns.pawn_attack ( diagonal_compass::sw, white_vars.pieces );

    /* Calculations for king attacks */
    bitboard white_king_attacks = white_vars.king.king_any_attack ( white_vars.sp, true );
    bitboard black_king_attacks = black_vars.king.king_any_attack ( black_vars.sp, true );

    /* Sum movability */
    white_vars.movability += white_pawn_pushes.popcount () + white_pawn_attacks_e.popcount () + white_pawn_attacks_w.popcount () + white_king_attacks.popcount ();
    black_vars.movability += black_pawn_pushes.popcount () + black_pawn_attacks_e.popcount () + black_pawn_attacks_w.popcount () + black_king_attacks.popcount ();

    /* Union attacks */
    white_vars.attack_union |= white_pawn_pushes | white_pawn_attacks_e | white_pawn_attacks_w | white_king_attacks;
    black_vars.attack_union |= black_pawn_pushes | black_pawn_attacks_e | black_pawn_attacks_w | black_king_attacks;

    /* Calculate if anyone is in check */
    white_vars.in_check |= static_cast<bool> ( black_vars.attack_union & white_vars.king );
    black_vars.in_check |= static_cast<bool> ( white_vars.attack_union & black_vars.king );



    /* CALCULATE THE VALUE */

    /* The running value */
    float value = 0.0;

    /* Basic piece costs */
    value += QUEEN  * ( !white_vars.queen.is_empty ()   - !black_vars.queen.is_empty () )
           + ROOK   * (  white_vars.rooks.popcount ()   - black_vars.rooks.popcount () )
           + BISHOP * (  white_vars.bishops.popcount () - black_vars.bishops.popcount () )
           + KNIGHT * (  white_vars.knights.popcount () - black_vars.knights.popcount () )
           + PAWN   * (  white_vars.pawns.popcount ()   - black_vars.pawns.popcount () );

    /* Movability bonus */
    value += MOVABILITY * ( white_vars.movability - black_vars.movability );

    /* Return the value */
    return value;
}



/* BOARD LOOKUP */



/** @name  find_color
 *
 * @brief  Determines the color of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return One of pcolor
 */
inline chess::pcolor chess::chessboard::find_color ( unsigned pos ) const noexcept
{
    bitboard mask { 1ull << pos };
    if ( bb ( pcolor::white ) & mask ) return pcolor::white; else
    if ( bb ( pcolor::black ) & mask ) return pcolor::black; else
    return pcolor::no_piece;
}

/** @name  find_type
 * 
 * @brief  Determines the type of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return One of ptype
 */
inline chess::ptype chess::chessboard::find_type ( unsigned pos ) const noexcept
{
    bitboard mask { 1ull << pos };
    if ( bb ( ptype::pawn   ) & mask ) return ptype::pawn;   else
    if ( bb ( ptype::king   ) & mask ) return ptype::king;   else
    if ( bb ( ptype::queen  ) & mask ) return ptype::queen;  else
    if ( bb ( ptype::bishop ) & mask ) return ptype::bishop; else
    if ( bb ( ptype::knight ) & mask ) return ptype::knight; else
    if ( bb ( ptype::rook   ) & mask ) return ptype::rook;   else
    return ptype::no_piece;
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_HPP_INCLUDED */