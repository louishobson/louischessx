/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * include/chess/chessboard_moves.hpp
 *
 * Inline implementation of move application and generation methods in include/chess/chessboard.h
 *
 */



/* HEADER GUARD */
#ifndef CHESS_CHESSBOARD_MOVES_HPP_INCLUDED
#define CHESS_CHESSBOARD_MOVES_HPP_INCLUDED



/* INCLUDES */
#include <chess/chessboard.h>



/* MAKE AND UNMAKE MOVES IMPLEMENTATION */



/** @name  make_move_internal
 * 
 * @brief  Apply a move. Assumes all the information about the move is correct and legal.
 * @param  move: The move to apply
 * @return The auxiliary information before the move was made
 */
inline chess::chessboard::aux_info_t chess::chessboard::make_move_internal ( const move_t& move ) chess_validate_throw
{
    /* Get the aux info */
    const aux_info_t aux = aux_info;

    /* Set the double push pos to zero (will be overriden if this is a pawn double push) */
    aux_info.double_push_pos = 0; 

    /* If this is a null move, return */
    if ( move.pt == ptype::no_piece ) return aux_info;

    /* Unset the original position of the piece */
    get_bb ( move.pc ).reset          ( move.from );
    get_bb ( move.pc, move.pt ).reset ( move.from );

    /* Set the new position of the piece */
    get_bb ( move.pc ).set          ( move.to );
    get_bb ( move.pc, move.pt ).set ( move.to ); 

    /* Check if this is an en passant capture, and remove the pawn */
    if ( move.en_passant_pos )
    {
        get_bb ( other_color ( move.pc ) ).reset              ( move.en_passant_pos );
        get_bb ( other_color ( move.pc ), ptype::pawn ).reset ( move.en_passant_pos );
    } else

    /* Else if this is a normal capture, remove any captured pieces */
    if ( move.capture_pt != ptype::no_piece )
    {
        get_bb ( other_color ( move.pc ) ).reset                  ( move.to );
        get_bb ( other_color ( move.pc ), move.capture_pt ).reset ( move.to );
    } else

    /* Else if the move is a kingside castle */
    if ( move.is_kingside_castle () )
    {
        /* Move the appropriate rook */
        get_bb ( move.pc ).reset              ( move.pc == pcolor::white ? 7 : 63 );
        get_bb ( move.pc, ptype::rook ).reset ( move.pc == pcolor::white ? 7 : 63 );
        get_bb ( move.pc ).set                ( move.pc == pcolor::white ? 5 : 61 );
        get_bb ( move.pc, ptype::rook ).set   ( move.pc == pcolor::white ? 5 : 61 );

        /* Set the new castling rights */
        set_castle_made ( move.pc );
    } else

    /* Else if the move is a queenside castle */
    if ( move.is_queenside_castle () )
    {
        /* Move the appropriate rook */
        get_bb ( move.pc ).reset              ( move.pc == pcolor::white ? 0 : 56 );
        get_bb ( move.pc, ptype::rook ).reset ( move.pc == pcolor::white ? 0 : 56 );
        get_bb ( move.pc ).set                ( move.pc == pcolor::white ? 3 : 59 );
        get_bb ( move.pc, ptype::rook ).set   ( move.pc == pcolor::white ? 3 : 59 );

        /* Set the new castling rights */
        set_castle_made ( move.pc );
    }    

    /* Set lost castling rights */
    if ( has_any_castling_rights ( move.pc ) && move.pt == ptype::king ) set_castle_lost ( move.pc );
    if ( has_kingside_castling_rights  ( pcolor::white ) && !bb ( pcolor::white, ptype::rook ).test (  7 ) ) set_kingside_castle_lost   ( pcolor::white );
    if ( has_queenside_castling_rights ( pcolor::white ) && !bb ( pcolor::white, ptype::rook ).test (  0 ) ) set_queenside_castle_lost  ( pcolor::white );
    if ( has_kingside_castling_rights  ( pcolor::black ) && !bb ( pcolor::black, ptype::rook ).test ( 63 ) ) set_kingside_castle_lost   ( pcolor::black );
    if ( has_queenside_castling_rights ( pcolor::black ) && !bb ( pcolor::black, ptype::rook ).test ( 56 ) ) set_queenside_castle_lost  ( pcolor::black );

    /* Promote pawn */
    if ( move.promote_pt != ptype::no_piece )
    {
        get_bb ( move.pc, move.promote_pt ).set ( move.to );
        get_bb ( move.pc, ptype::pawn ).reset   ( move.to );
    }

    /* If this move is a pawn double push, set double_push_pos to the final position of this pawn.
     * | to - from | == 16 if the move is a double push.
     */
    if ( move.pt == ptype::pawn && std::abs ( move.to - move.from ) == 16 ) aux_info.double_push_pos = move.to;

    /* Sanity check */
    sanity_check_bbs ();

    /* Return the old auxiliary info */
    return aux;
}

/** @name  unmake_move_internal
 * 
 * @brief  Unmake a move. Assumes that the move was made immediately before this undo function.
 * @param  move: The move to undo
 * @param  aux: The auxiliary information from before the move was first made
 * @return void
 */
inline void chess::chessboard::unmake_move_internal ( const move_t& move, const aux_info_t aux ) chess_validate_throw
{
    /* Reset aux info */
    aux_info = aux;

    /* If the move was a null move, return */
    if ( move.pt == ptype::no_piece ) return;

    /* Unpromote pawn */
    if ( move.promote_pt != ptype::no_piece )
    {
        get_bb ( move.pc, move.promote_pt ).reset ( move.to );
        get_bb ( move.pc, ptype::pawn ).set ( move.to );
    }

    /* Check for if the move was a kingside castle */
    if ( move.is_kingside_castle () )
    {
        /* Unmove the appropriate rook */
        get_bb ( move.pc ).reset              ( move.pc == pcolor::white ? 5 : 61 );
        get_bb ( move.pc, ptype::rook ).reset ( move.pc == pcolor::white ? 5 : 61 );
        get_bb ( move.pc ).set                ( move.pc == pcolor::white ? 7 : 63 );
        get_bb ( move.pc, ptype::rook ).set   ( move.pc == pcolor::white ? 7 : 63 );
    } else

    /* Else if the move was a queenside castle */
    if ( move.is_queenside_castle () )
    {
        /* Unmove the appropriate rook */
        get_bb ( move.pc ).reset              ( move.pc == pcolor::white ? 3 : 59 );
        get_bb ( move.pc, ptype::rook ).reset ( move.pc == pcolor::white ? 3 : 59 );
        get_bb ( move.pc ).set                ( move.pc == pcolor::white ? 0 : 56 );
        get_bb ( move.pc, ptype::rook ).set   ( move.pc == pcolor::white ? 0 : 56 );
    } else

    /* Else if the move was an en passant capture, replace the captured pawn */
    if ( move.en_passant_pos )
    {
        get_bb ( other_color ( move.pc ) ).set              ( move.en_passant_pos );
        get_bb ( other_color ( move.pc ), ptype::pawn ).set ( move.en_passant_pos );
    } else

    /* Else if the move was a capture, replace captured pieces */
    if ( move.capture_pt != ptype::no_piece )
    {
        get_bb ( other_color ( move.pc ) ).set                  ( move.to );
        get_bb ( other_color ( move.pc ), move.capture_pt ).set ( move.to );
    }

    /* Unset the new position of the piece */
    get_bb ( move.pc ).reset ( move.to );
    get_bb ( move.pc, move.pt ).reset ( move.to ); 

    /* Reset the original position of the piece */
    get_bb ( move.pc ).set ( move.from );
    get_bb ( move.pc, move.pt ).set ( move.from );

    /* Sanity check */
    sanity_check_bbs ();
}



/* CASTLE LEGALLITY DETECTION */



/** @name  can_castle, can_kingside_castle, can_queenside_castle
 * 
 * @brief  Gets information as to whether a color can legally castle given the current state
 * @param  pc: One of pcolor. Undefined behaviour if is no_piece.
 * @param  check_info: The check info for the king in its current position
 * @return boolean
 */
inline bool chess::chessboard::can_kingside_castle ( const pcolor pc, const check_info_t& check_info ) const chess_validate_throw
{
    /* Return false if castling is not availible */
    if ( !has_kingside_castling_rights ( pc ) ) return false;

    /* Return if is in check */
    if ( check_info.check_count ) return false;

    /* Return if the space inbetween the king and rook is not empty */
    const bitboard empty_squares { bitboard::masks::kingside_castle_empty_squares & ( pc == pcolor::white ? bitboard::masks::rank_1 : bitboard::masks::rank_8 ) };
    if ( bb () & empty_squares ) return false;

    /* Get the squares that must be safe */
    bitboard safe_squares { bitboard::masks::kingside_castle_safe_squares & ~bitboard::masks::king_opening & ( pc == pcolor::white ? bitboard::masks::rank_1 : bitboard::masks::rank_8 ) };

    /* Iterate through the safe squares and return if in check on any of them */
    while ( safe_squares )
    {
        const int pos = safe_squares.trailing_zeros ();
        safe_squares.reset ( pos );
        if ( is_protected ( other_color ( pc ), pos ) ) return false;
    }

    /* Castle is possible, so return true */
    return true;
}
inline bool chess::chessboard::can_queenside_castle ( const pcolor pc, const check_info_t& check_info ) const chess_validate_throw
{
    /* Return false if castling is not availible */
    if ( !has_queenside_castling_rights ( pc ) ) return false;

    /* Return if is in check */
    if ( check_info.check_count ) return false;

    /* Return if the space inbetween the king and rook is not empty */
    const bitboard empty_squares { bitboard::masks::queenside_castle_empty_squares & ( pc == pcolor::white ? bitboard::masks::rank_1 : bitboard::masks::rank_8 ) };
    if ( bb () & empty_squares ) return false;

    /* Get the squares that must be safe */
    bitboard safe_squares { bitboard::masks::queenside_castle_safe_squares & ~bitboard::masks::king_opening & ( pc == pcolor::white ? bitboard::masks::rank_1 : bitboard::masks::rank_8 ) };

    /* Iterate through the safe squares and return if in check on any of them */
    while ( safe_squares )
    {
        const int pos = safe_squares.trailing_zeros ();
        safe_squares.reset ( pos );
        if ( is_protected ( other_color ( pc ), pos ) ) return false;
    }

    /* Castle is possible, so return true */
    return true;
}



/* MOVE CALCULATIONS */



/** @name  get_move_set
 * 
 * @brief  Gets the move set for a given type and position of piece
 * @param  pc: The color who owns the pawn
 * @param  pt: The type of the piece
 * @param  pos: The position of the pawn
 * @param  check_info: The check info for pc
 * @return A bitboard containing the possible moves for the piece in question
 */
inline chess::bitboard chess::chessboard::get_move_set ( pcolor pc, ptype pt, int pos, const check_info_t& check_info ) chess_validate_throw
{
    /* Switch depending on pt */
    switch ( pt )
    {
    case ptype::pawn   : return get_pawn_move_set    ( pc, pos, check_info );
    case ptype::knight : return get_knight_move_set  ( pc, pos, check_info );
    case ptype::bishop : return get_sliding_move_set ( pc, ptype::bishop, pos, check_info );
    case ptype::rook   : return get_sliding_move_set ( pc, ptype::rook,   pos, check_info );
    case ptype::queen  : return get_sliding_move_set ( pc, ptype::queen,  pos, check_info );
    case ptype::king   : return get_king_move_set    ( pc, check_info );
    }

    /* Throw if validation is enabled, otherwise return an empty bitboard */
#if CHESS_VALIDATE
    throw std::runtime_error { "Recieved invalid piece type in get_move_set" };
#else
    return bitboard {};
#endif
}

/** @name  get_pawn_move_set
 * 
 * @brief  Gets the move set for a pawn
 * @param  pc: The color who owns the pawn
 * @param  pos: The position of the pawn
 * @param  check_info: The check info for pc
 * @return A bitboard containing the possible moves for the pawn in question
 */
inline chess::bitboard chess::chessboard::get_pawn_move_set ( const pcolor pc, const int pos, const check_info_t& check_info ) chess_validate_throw
{
    /* Get the pawn in question */
    const bitboard pawn { singleton_bitboard ( pos ) };

    /* Create an empty set of moves */
    bitboard moves;

    /* If is on a straight pin vector, cannot be a valid pawn attack */
    if ( pawn.is_disjoint ( check_info.straight_pin_vectors ) )
    {
        /* Get the attacks, and ensure that they protected the king */
        bitboard attacks = ( pc == pcolor::white ? pawn.pawn_any_attack_n ( bb ( other_color ( pc ) ) ) : pawn.pawn_any_attack_s ( bb ( other_color ( pc ) ) ) ) & check_info.check_vectors_dep_check_count;

        /* Look for an en passant opportinity. 
         * There must have been a double pushed pawn on the previous move.
         * The pawns must be adjacent and on the same rank.
         */
        if ( aux_info.double_push_pos && std::abs ( pos - aux_info.double_push_pos ) == 1 && pos / 8 == aux_info.double_push_pos / 8 ) 
        {
            /* Create a move for the en passant capture */
            const move_t ep_move { pc, ptype::pawn, ptype::pawn, ptype::no_piece, pos, aux_info.double_push_pos + ( pc == pcolor::white ? +8 : -8 ), aux_info.double_push_pos, 0 }; 

            /* Only add the en passant capture to the attack set if making the capture does not leave the king in check */
            const aux_info_t aux = make_move_internal ( ep_move );
            if ( !is_in_check ( pc ) ) attacks |= singleton_bitboard ( ep_move.to );
            unmake_move_internal ( ep_move, aux );
        }

        /* If is on a diagonal pin vector, ensure the captures stayed on the pin vector */
        if ( pawn & check_info.diagonal_pin_vectors ) attacks &= check_info.diagonal_pin_vectors;

        /* Union moves */
        moves |= attacks;                
    }

    /* If is on a diagonal pin vector, cannot be a valid pawn push */
    if ( pawn.is_disjoint ( check_info.diagonal_pin_vectors ) )
    {
        /* Get the pushes, and ensure that they protected the king */
        bitboard pushes = ( pc == pcolor::white ? pawn.pawn_push_n ( ~bb () ) : pawn.pawn_push_s ( ~bb () ) ) & check_info.check_vectors_dep_check_count;

        /* If is on a straight pin vector, ensure the push stayed on the pin vector */
        if ( pawn & check_info.straight_pin_vectors ) pushes &= check_info.straight_pin_vectors;

        /* Union moves */
        moves |= pushes;
    }

    /* Return the moves */
    return moves;
}

/** @name  get_knight_move_set
 * 
 * @brief  Gets the move set for a knight
 * @param  pc: The color who owns the knight
 * @param  pos: The position of the knight
 * @param  check_info: The check info for pc
 * @return A bitboard containing the possible moves for the knight in question
 */
inline chess::bitboard chess::chessboard::get_knight_move_set ( const pcolor pc, const int pos, const check_info_t& check_info ) chess_validate_throw
{
    /* Get the knight in question */
    const bitboard knight = singleton_bitboard ( pos );

    /* If the knight is pinned, return an empty attack set */
    if ( knight & check_info.pin_vectors ) return bitboard {};

    /* Return the attacks, ensuring they protected the king */
    return bitboard::knight_attack_lookup ( pos ) & ~bb ( pc ) & check_info.check_vectors_dep_check_count;
}

/** @name  get_sliding_move_set
 * 
 * @brief  Gets the move set for a sliding
 * @param  pc: The color who owns the sliding piece
 * @param  pt: The type of the sliding piece, undefined if not a sliding piece
 * @param  pos: The position of the sliding piece
 * @param  check_info: The check info for pc
 * @return A bitboard containing the possible moves for the sliding in question
 */
inline chess::bitboard chess::chessboard::get_sliding_move_set ( const pcolor pc, const ptype pt, const int pos, const check_info_t& check_info ) chess_validate_throw
{
    /* Get the piece in question */
    const bitboard pt_bb = singleton_bitboard ( pos );

    /* Get the straight and diagonal attack lookups */
    bitboard straight_attack_lookup = bitboard::straight_attack_lookup ( pos );
    bitboard diagonal_attack_lookup = bitboard::diagonal_attack_lookup ( pos );

    /* If the piece is pinned, then it is possible the piece may be immobile.
     * First check if it is on a straight pin vector.
     */
    if ( pt_bb & check_info.straight_pin_vectors ) [[ unlikely ]]
    {
        /* If in check, or is a bishop, return an empty move set */
        if ( check_info.check_count || ( pt == ptype::bishop ) ) return bitboard {};

        /* Now we know the pinned piece can move, restrict the propagators accordingly */
        straight_attack_lookup &= check_info.straight_pin_vectors;
        diagonal_attack_lookup.empty ();
    } else

    /* Else check if is on a diagonal pin vector */
    if ( pt_bb & check_info.diagonal_pin_vectors ) [[ unlikely ]]
    {
        /* If in check, or is a rook, return an empty move set */
        if ( check_info.check_count || ( pt == ptype::rook ) ) return bitboard {};

        /* Now we know the pinned piece can move, restrict the propagators accordingly */
        diagonal_attack_lookup &= check_info.diagonal_pin_vectors;
        straight_attack_lookup.empty ();
    }

    /* Set moves initially to empty */
    bitboard moves;

    /* If is not a bishop, apply a straight flood span.
     * If is not a rook, apply a diagonal flood span.
     */
    if ( pt != ptype::bishop ) moves |= pt_bb.straight_flood_span ( ~bb () & straight_attack_lookup, ~bb ( pc ) & straight_attack_lookup );
    if ( pt != ptype::rook   ) moves |= pt_bb.diagonal_flood_span ( ~bb () & diagonal_attack_lookup, ~bb ( pc ) & diagonal_attack_lookup );

    /* Ensure that moves protected the king */
    moves &= check_info.check_vectors_dep_check_count;

    /* Return the move set */
    return moves;
}

/** @name  get_king_move_set
 *  
 * @brief  Gets the move set for the king.
 *         Note that although is non-const, a call to this function will leave the board unmodified.
 * @param  pc: The color who owns the king
 * @param  check_info: The check info for pc
 * @return A bitboard containing the possible moves for the king in question
 */
inline chess::bitboard chess::chessboard::get_king_move_set ( const pcolor pc, const check_info_t& check_info ) chess_validate_throw
{
    /* Get the king */
    const bitboard king = bb ( pc, ptype::king );

    /* Lookup the king moves */
    bitboard moves = bitboard::king_attack_lookup ( king.trailing_zeros () ) & ~bb ( pc );

    /* Unset the king */
    get_bb ( pc ) &= ~king;
    get_bb ( pc, ptype::king ).empty ();

    /* Iterate through the moves and make sure that they do not lead to check */
    for ( bitboard moves_temp = moves; moves_temp; )
    {
        /* Get the next test position */
        const int test_pos = moves_temp.trailing_zeros ();
        moves_temp.reset ( test_pos );

        /* If is protected, reset in attacks */
        moves.reset_if ( test_pos, is_protected ( other_color ( pc ), test_pos ) );
    }

    /* Reset the king */
    get_bb ( pc )              |= king;
    get_bb ( pc, ptype::king ) |= king;

    /* Add castling moves */
    if ( can_kingside_castle  ( pc, check_info ) ) moves |= king.shift ( compass::e ).shift ( compass::e );
    if ( can_queenside_castle ( pc, check_info ) ) moves |= king.shift ( compass::w ).shift ( compass::w );

    /* Return the moves */
    return moves;
}



/* SANITY CHECKS */



/** @name  sanity_check_bbs
 * 
 * @brief  Sanity check the bitboards describing the board state. 
 *         If any cell is occupied by multiple pieces, or ptype::any_piece bitboards are not correct, an exception is thrown.
 *         Does nothing if CHESS_VALIDATE is not set to true.
 * @return void
 */
inline void chess::chessboard::sanity_check_bbs () const chess_validate_throw
{
    /* Only if validation is enabled */
#if CHESS_VALIDATE

    /* Iterate through each color */
    pcolor pc = pcolor::white;
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 2
    for ( int i = 0; i < 2; ++i )
    {
        /* Get the bitboard for this color */
        bitboard all = bb ( pc );

        /* Iterate through each type */
        #pragma clang loop unroll ( full )
        #pragma GCC unroll 6
        for ( const ptype pt : ptype_inc_value )
        {
            /* Switch the bits in all */
            all ^= bb ( pc, pt );

            /* Test for errors */
            if ( all & bb ( pc, pt ) ) throw std::runtime_error { "Sanity check failed." };
        }

        /* Throw if all has any set bits left */
        if ( all ) throw std::runtime_error { "Sanity check failed." };

        /* Get the next color */
        pc = other_color ( pc );
    }

#endif
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_MOVES_HPP_INCLUDED */