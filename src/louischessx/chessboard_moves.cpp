/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/chessboard_moves.cpp
 * 
 * Implementation of move methods in include/chess/chessboard.h
 * 
 */



/* INCLUDES */
#include <louischessx/chessboard.h>



/* CHECKING MOVES */



/** @name  check_move_is_valid
 * 
 * @brief  Throws if a move is invalid to apply to the current state
 * @param  move: The move to test
 * @return void
 */
void chess::chessboard::check_move_is_valid ( const move_t& move )
{
    /* Check that the piece the move refers to exists */
    if ( !bb ( move.pc, move.pt ).test ( move.from ) ) throw chess_input_error { "Invalid move initial position or type in check_move_is_valid ()." };

    /* Check that the move is legal */
    if ( !get_move_set ( move.pc, move.pt, move.from, get_check_info ( move.pc ) ).test ( move.to ) ) throw chess_input_error { "Illegal move final position in check_move_is_valid ()." };

    /* Check, if the move is en passant, that it is valid */
    if ( move.pt == ptype::pawn && move.pc == aux_info.en_passant_color && move.to == aux_info.en_passant_target ) 
    {
        /* Check that the capture type is correct */
        if ( move.capture_pt != ptype::pawn ) throw chess_input_error { "Invalid capture type or en passant pos (expected en passant) in check_move_is_valid ()." };
    } else

    /* Else this is not an en passant capture */
    {
        /* Get if this move is a capture */
        if ( ptype exp_capture_pt = find_type ( other_color ( move.pc ), move.to ); exp_capture_pt != ptype::no_piece )
        {
            /* Throw if the capture type is not correct */
            if ( move.capture_pt != exp_capture_pt ) throw chess_input_error { "Invalid capture type in check_move_is_valid ()." };
        } else

        /* This is a non-capture, so ensure that the capture type is no_piece */
        if ( move.capture_pt != ptype::no_piece ) throw chess_input_error { "Invalid capture type in check_move_is_valid ()." };
    }

    /* Test if the move should be a pawn promotion and hence check the promote_pt is valid */
    if ( move.pt == ptype::pawn && ( move.pc == pcolor::white ? move.to >= 56 : move.to < 8 ) )
    {
        /* Move should promote, so check the promotion type is valid */
        if ( move.promote_pt != ptype::knight && move.promote_pt != ptype::bishop && move.promote_pt != ptype::rook && move.promote_pt != ptype::queen ) 
            throw chess_input_error { "Invalid promotion type (move should promote) in check_move_is_valid ()." };
    } else
    {
        /* Move should not promote, so check the promotion type is no_piece */
        if ( move.promote_pt != ptype::no_piece ) throw chess_input_error { "Invalid promotion type (move should not promote) in check_move_is_valid ()." };
    }

    /* Don't worry about the check, checkmate, stalemate or draw flags */
}



/* MAKING MOVES */



/** @name  make_move_internal
 * 
 * @brief  Apply a move. Assumes all the information about the move is correct and legal.
 * @param  move: The move to apply
 * @return void
 */
void chess::chessboard::make_move_internal ( const move_t& move )
{
    /* Get the aux info */
    const aux_info_t aux = aux_info;

    /* If this is a null move, reset en passant variables, add to the history, sanity check and return */
    if ( move.pt == ptype::no_piece )
    {
        aux_info.en_passant_target = -1; aux_info.en_passant_color = pcolor::no_piece;
        game_state_history.emplace_back ( * this, move.pc );
        sanity_check_bbs ( move.pc );
        return;
    }

    /* Unset the original position of the piece */
    get_bb ( move.pc ).reset          ( move.from );
    get_bb ( move.pc, move.pt ).reset ( move.from );

    /* Set the new position of the piece */
    get_bb ( move.pc ).set          ( move.to );
    get_bb ( move.pc, move.pt ).set ( move.to ); 

    /* Check if this is an en passant capture, and remove the pawn */
    if ( move.pt == ptype::pawn && move.pc == aux_info.en_passant_color && move.to == aux_info.en_passant_target )
    {
        get_bb ( other_color ( move.pc ) ).reset              ( move.en_passant_capture_pos () );
        get_bb ( other_color ( move.pc ), ptype::pawn ).reset ( move.en_passant_capture_pos () );
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

    /* If this move is a pawn double push, set the en passant target square and color */
    if ( move.pt == ptype::pawn && move.to - move.from == +16 ) { aux_info.en_passant_target = move.to - 8; aux_info.en_passant_color = pcolor::black; } else
    if ( move.pt == ptype::pawn && move.to - move.from == -16 ) { aux_info.en_passant_target = move.to + 8; aux_info.en_passant_color = pcolor::white; } else
    
    /* Else reset en passant target square and color to -1 and no_piece */
    { aux_info.en_passant_target = -1; aux_info.en_passant_color = pcolor::no_piece; }

    /* Push the new state to the history */
    game_state_history.emplace_back ( * this, move.pc );

    /* Sanity check */
    sanity_check_bbs ( move.pc );
}

/** @name  unmake_move_internal
 * 
 * @brief  Unmake the last made move.
 * @return void
 */
void chess::chessboard::unmake_move_internal ()
{
    /* Pop this state from the history */
    game_state_history.pop_back ();

    /* Copy over the color bitboards */
    get_bb ( pcolor::white ) = game_state_history.back ().bb ( pcolor::white );
    get_bb ( pcolor::black ) = game_state_history.back ().bb ( pcolor::black );

    /* Copy over the rest */
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 6
    for ( ptype pt : ptype_inc_value )
    {
        get_bb ( pcolor::white, pt ) = game_state_history.back ().bb ( pcolor::white, pt );
        get_bb ( pcolor::black, pt ) = game_state_history.back ().bb ( pcolor::black, pt );
    }

    /* Copy over aux */
    aux_info = game_state_history.back ().aux_info;
}



/* CASTLE LEGALLITY DETECTION */



/** @name  can_castle, can_kingside_castle, can_queenside_castle
 * 
 * @brief  Gets information as to whether a color can legally castle given the current state
 * @param  pc: One of pcolor. Undefined behaviour if is no_piece.
 * @param  check_info: The check info for the king in its current position
 * @return boolean
 */
bool chess::chessboard::can_kingside_castle ( const pcolor pc, const check_info_t& check_info ) const chess_validate_throw
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
bool chess::chessboard::can_queenside_castle ( const pcolor pc, const check_info_t& check_info ) const chess_validate_throw
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
chess::bitboard chess::chessboard::get_move_set ( pcolor pc, ptype pt, int pos, const check_info_t& check_info )
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
    throw chess_internal_error { "Recieved invalid piece type in get_move_set" };
#else
    return bitboard {};
#endif
}



/** @name  has_mobility
 * 
 * @brief  Gets whether a color has any mobility
 * @param  pc: The color to check for mobility
 * @param  check_info: The check info for pc.
 * @return boolean
 */
bool chess::chessboard::has_mobility ( pcolor pc, const check_info_t& check_info )
{
    /* Iterate through the pieces */
    for ( const ptype pt : ptype_inc_value ) 
    {
        /* Iterate through pieces */
        for ( bitboard pieces = bb ( pc, pt ); pieces; )
        {
            /* Get a position of a piece and reset it */
            const int pos = pieces.trailing_zeros ();
            pieces.reset ( pos );

            /* Return true if the move set is non-empty */
            if ( get_move_set ( pc, pt, pos, check_info ) ) return true;
        }
    }

    /* No moves, so return false */
    return false;
}



/** @name  get_pawn_move_set
 * 
 * @brief  Gets the move set for a pawn
 * @param  pc: The color who owns the pawn
 * @param  pos: The position of the pawn
 * @param  check_info: The check info for pc
 * @return A bitboard containing the possible moves for the pawn in question
 */
chess::bitboard chess::chessboard::get_pawn_move_set ( const pcolor pc, const int pos, const check_info_t& check_info )
{
    /* Get the pawn in question */
    const bitboard pawn { singleton_bitboard ( pos ) };

    /* Create an empty set of moves */
    bitboard moves;

    /* Get the general attacks */
    const bitboard general_attacks = ( pc == pcolor::white ? pawn.pawn_any_attack_n () : pawn.pawn_any_attack_s () );

    /* If is on a straight pin vector, cannot be a valid pawn attack */
    if ( pawn.is_disjoint ( check_info.straight_pin_vectors ) )
    {
        /* Get the legal attacks, not including en passant capture */
        bitboard attacks = general_attacks & bb ( other_color ( pc ) ) & check_info.check_vectors_dep_check_count;

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

    /* Look for en passant capture, and if it does not cause check, add it to moves */
    const int en_passant_target = aux_info.en_passant_target;
    if ( general_attacks.only_if ( pc == aux_info.en_passant_color ).test ( en_passant_target ) ) 
    {

        make_move_internal ( move_t { pc, ptype::pawn, ptype::pawn, ptype::no_piece, pos, en_passant_target } );
        if ( !is_in_check ( pc ) ) moves.set ( en_passant_target );
        unmake_move_internal ();
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
chess::bitboard chess::chessboard::get_knight_move_set ( const pcolor pc, const int pos, const check_info_t& check_info )
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
chess::bitboard chess::chessboard::get_sliding_move_set ( const pcolor pc, const ptype pt, const int pos, const check_info_t& check_info )
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
chess::bitboard chess::chessboard::get_king_move_set ( const pcolor pc, const check_info_t& check_info )
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
 * @param  _last_pc: The player who last moved
 * @return void
 */
void chess::chessboard::sanity_check_bbs ( const pcolor _last_pc ) const chess_validate_throw
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
            if ( all & bb ( pc, pt ) ) throw chess_internal_error { "Sanity check failed." };
        }

        /* Throw if all has any set bits left */
        if ( all ) throw chess_internal_error { "Sanity check failed." };

        /* Get the next color */
        pc = other_color ( pc );
    }

    /* Check the most recent history is correct */
    if ( game_state_t { * this, _last_pc } != game_state_history.back () ) throw chess_internal_error { "Sanity check failed." };

#endif
}