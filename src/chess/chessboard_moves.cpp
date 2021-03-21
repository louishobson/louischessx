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
#include <chess/chessboard.h>



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
    if ( !bb ( move.pc, move.pt ).test ( move.from ) ) throw std::runtime_error { "Invalid move initial position or type in check_move_is_valid ()." };

    /* Check that the move is legal */
    if ( !get_move_set ( move.pc, move.pt, move.from, get_check_info ( move.pc ) ).test ( move.to ) ) throw std::runtime_error { "Illegal move final position in check_move_is_valid ()." };

    /* Get whether this move is an en passant capture */
    if ( aux_info.double_push_pos && move.pt == ptype::pawn && move.to - aux_info.double_push_pos == ( move.pc == pcolor::white ? +8 : -8 ) ) 
    {
        /* Check that the capture type and en passant pos is correct */
        if ( move.capture_pt != ptype::pawn || move.en_passant_pos != aux_info.double_push_pos ) throw std::runtime_error { "Invalid capture type or en passant pos (expected en passant) in check_move_is_valid ()." };
    } else

    /* Else this is not an en passant capture */
    {
        /* En passant pos should be 0 */
        if ( move.en_passant_pos ) throw std::runtime_error { "Invalid en passant pos (unexpected en passant) in check_move_is_valid ()." };

        /* Get if this move is a capture */
        if ( ptype exp_capture_pt = find_type ( other_color ( move.pc ), move.to ); exp_capture_pt != ptype::no_piece )
        {
            /* Throw if the capture type is not correct */
            if ( move.capture_pt != exp_capture_pt ) throw std::runtime_error { "Invalid capture type in check_move_is_valid ()." };
        } else

        /* This is a non-capture, so ensure that the capture type is no_piece */
        if ( move.capture_pt != ptype::no_piece ) throw std::runtime_error { "Invalid capture type in check_move_is_valid ()." };
    }

    /* Test if the move should be a pawn promotion and hence check the promote_pt is valid */
    if ( move.pt == ptype::pawn && ( move.pc == pcolor::white ? move.to >= 56 : move.to < 8 ) )
    {
        /* Move should promote, so check the promotion type is valid */
        if ( move.promote_pt != ptype::knight && move.promote_pt != ptype::bishop && move.promote_pt != ptype::rook && move.promote_pt != ptype::queen ) 
            throw std::runtime_error { "Invalid promotion type (move should promote) in check_move_is_valid ()." };
    } else
    {
        /* Move should not promote, so check the promotion type is no_piece */
        if ( move.promote_pt != ptype::no_piece ) throw std::runtime_error { "Invalid promotion type (move should not promote) in check_move_is_valid ()." };
    }

    /* Make the move */
    make_move_internal ( move );

    /* Ensure the check and checkmate flags are correct */
    if ( move.check != is_in_check ( other_color ( move.pc ) ) ) throw std::runtime_error { "Incorrct check flag in check_move_is_valid ()." }; 
    if ( move.checkmate != ( evaluate ( move.pc ) == 10000 ) )   throw std::runtime_error { "Incorrct checkmate flag in check_move_is_valid ()." };

    /* Unmake the move */
    unmake_move_internal ();
}