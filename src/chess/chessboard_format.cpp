/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/chessboard_format.cpp
 * 
 * Implementation of formatting methods in include/chess/chessboard.h
 * 
 */



/* INCLUDES */
#include <chess/chessboard.h>



/* TYPE CHARACTER CONVERSION */



/** @name  ptype_to_character
 * 
 * @brief  Convert a ptype to a character
 * @param  pt: The character to convert
 * @return char
 */
char chess::chessboard::ptype_to_character ( ptype pt ) noexcept
{
    /* Cast and return using the cast as an index */
    return piece_chars [ cast_penum ( pt ) ];
}

/** @name  character_to_ptype
 * 
 * @brief  Convert a character to a ptype. Return no_piece for an unknown character.
 * @param  c: The character to convert
 * @return ptype
 */
chess::ptype chess::chessboard::character_to_ptype ( char c ) noexcept
{
    /* Look for the character in piece_chars. Using + 7 as an upper bound means that unknown characters will end on no_piece */
    const char * pt_ptr = std::find ( piece_chars, piece_chars + 7, c );

    /* Cast the difference and return */
    return static_cast<ptype> ( pt_ptr - piece_chars );
}



/* SIMPLE BOARD FORMATTING */



/** @name  simple_format_board
 * 
 * @brief  Create a simple representation of the board.
 *         Lower-case letters mean black, upper case white.
 * @return string
 */
std::string chess::chessboard::simple_format_board () const
{
    /* Add to string one bit at a time.
     * i ^ 56 changes the endianness of i, such that the top row is read first.
     * Multiplying by 2 skips the spaces inbetween cells.
     */
    std::string out ( 128, ' ' );
    for ( int i = 0; i < 64; ++i ) 
    {
        /* Get the piece color */
        const pcolor pc = find_color ( i ^ 56 ); 

        /* If there is no piece, output a dot, otherwise find the correct character to output */
        if ( pc == pcolor::no_piece ) out [ i * 2 ] = '.'; else
        {
            /* Find the type */
            const ptype pt = find_type ( pc, i ^ 56 );
            
            /* Add the right character */
            out [ i * 2 ] = ptype_to_character ( pt );
            if ( pc == pcolor::black ) out [ i * 2 ] = std::tolower ( out [ i * 2 ] );
        }
    
        /* Possibly add a newline */
        if ( ( i & 7 ) == 7 ) out [ i * 2 + 1 ] = '\n';
    };

    /* Return the formatted string */
    return out;
}



/* FIDE MOVE SERIALIZATION */



/** @name  fide_serialize_move
 * 
 * @brief  Take a move valid for this position and serialize it based on the FIDE standard
 * @param  move: The move to serialize
 * @return string
 */
std::string chess::chessboard::fide_serialize_move ( const move_t& move ) const
{
    /* Check the move is value */
    check_move_is_valid ( move );

    /* Check if the move is a castling move */
    if ( move.is_kingside_castle  () ) return "0-0"; 
    if ( move.is_queenside_castle () ) return "0-0-0";

    /* Get the check info */
    const check_info_t check_info = get_check_info ( move.pc );

    /* Iterate through the pieces of the moving color and determine how many of them are able to make the move */
    bool piece_conflict = false, files_conflict = false, ranks_conflict = false;
    for ( bitboard pieces = bb ( move.pc, move.pt ); pieces; )
    {
        /* Get the position of the next piece and unset it in pieces */
        int pos = pieces.trailing_zeros ();
        pieces.reset ( pos );

        /* Continue if this is the piece move refers to */
        if ( pos == move.from ) continue;

        /* Detect if the final position of the move is also present in this piece */
        if ( get_move_set ( move.pc, move.pt, pos, check_info ).test ( move.to ) )
        {
            /* Set that there is a conflicting piece */
            piece_conflict = true;

            /* Detect if this piece's file or rank conflicts with the piece the move refers to */
            if ( pos % 8 == move.from % 8 ) files_conflict = true; else
            if ( pos / 8 == move.from / 8 ) ranks_conflict = true;
        }
    }

    /* Hence get the departure position name */
    std::string from_cell_name;
    if ( piece_conflict )
    {
        if ( !files_conflict ) from_cell_name = bitboard::name_cell ( move.from ).at ( 0 ); else
        if ( !ranks_conflict ) from_cell_name = bitboard::name_cell ( move.from ).at ( 1 ); else
                               from_cell_name = bitboard::name_cell ( move.from );
    }

    /* Create the output string */
    std::string out = 
        /* If the piece is not a pawn, name which piece is moving */
        ( move.pt != ptype::pawn ? std::string { ptype_to_character ( move.pt ) } : "" ) +
        /* Add the departure position */
        from_cell_name +
        /* If the move is a capture, add an x */
        ( move.capture_pt != ptype::no_piece ? "x" : "" ) +
        /* Add the destination position */
        bitboard::name_cell ( move.to ) +
        /* Add the promotion type if necessary */
        ( move.promote_pt != ptype::no_piece ? std::string { ptype_to_character ( move.capture_pt ) } : "" ) +
        /* Add a '#' for checkmate or '+' for check */
        ( move.checkmate ? "#" : ( move.check ? "+"  : "" ) );

    /* Return the output */
    return out;
}

/** @name  fide_deserialize_move
 * 
 * @brief  Take a string describing a move for this position and deserialize it based on the FIDE standard
 * @param  pc: The color that is to make the move
 * @param  desc: The description to deserialize
 * @return move_t
 */
chess::chessboard::move_t chess::chessboard::fide_deserialize_move ( const pcolor pc, std::string desc ) const
{
    /* CASTLING MOVE */

    /* Look for a castling move */
    if ( desc == "O-O"   ) return move_t { pc, ptype::king, ptype::no_piece,  ptype::no_piece, ( pc == pcolor::white ? 4 : 60 ), ( pc == pcolor::white ? 6 : 62 ) };
    if ( desc == "O-O-O" ) return move_t { pc, ptype::king, ptype::no_piece,  ptype::no_piece, ( pc == pcolor::white ? 4 : 60 ), ( pc == pcolor::white ? 2 : 58 ) };



    /* SETUP AND REGEX SEARCH */

    /* Get the check info */
    const check_info_t check_info = get_check_info ( pc );

    /* Create an empty move */
    move_t move { pc };

    /* Reverse the move description.
     * The regex works in reverse, so that the destination position is reached first, rather than any disambiguation characters.
     */
    std::reverse ( desc.begin (), desc.end () );

    /* Create the regex to extract the information */
    std::regex move_regex { "([NBRQ]\?\?)([1-8][a-h])(x\?\?)([1-8]\?\?)([a-h]\?\?)([PNBRQK]\?\?)$" };
    
    /* Run the search */
    std::smatch move_match;
    std::regex_search ( desc, move_match, move_regex );
    
    /* Check that the number of submatches is exactly 7 (the number of groups plus one for the entire match) */
    if ( move_match.size () != 7 ) throw std::runtime_error { "Could not format move description in fide_deserialize_move ()." };

    /* An iterator to the current submatch in focus */
    auto move_submatch_it = std::reverse_iterator { move_match.end () };



    /* EXTRACT INFO */

    /* See if there is a promotion type */
    if ( move_submatch_it->length () ) move.pt = character_to_ptype ( move_submatch_it->str ().at ( 0 ) );
    else move.pt = ptype::pawn; ++move_submatch_it;

    /* Get the known file or rank from the disambiguation characters.
     * Rank comes first since the input has been reversed.
     */
    const int known_rank = ( move_submatch_it->length () ? move_submatch_it->str ().at ( 0 ) - '1' : -1 ); ++move_submatch_it;
    const int known_file = ( move_submatch_it->length () ? move_submatch_it->str ().at ( 0 ) - 'a' : -1 ); ++move_submatch_it;

    /* Get if there is a capture character */
    const bool capture_char = move_submatch_it++->length ();

    /* Get the destination position */
    move.to = bitboard::cell_pos_reversed ( * move_submatch_it++ );

    /* Get promotion type, if given */
    if ( move_submatch_it->length () ) move.promote_pt = character_to_ptype ( move_submatch_it->str ().at ( 0 ) ); ++move_submatch_it;

    /* Determine the capture type from the destination position */
    move.capture_pt = find_type ( other_color ( move.pc ), move.to );

    /* Determine the en passant pos from the destination position */
    if ( aux_info.double_push_pos && move.pt == ptype::pawn && move.capture_pt == ptype::no_piece && move.to == aux_info.double_push_pos + ( move.pc == pcolor::white ? +8 : -8 ) )
        { move.capture_pt = ptype::pawn; move.en_passant_pos = aux_info.double_push_pos; }



    /* GET DEPARTURE POSITION */

    /* A bitboard to store the possible departure positions */
    bitboard from_bb;

    /* Iterate through the pieces of the moving color and find any that can make the move */
    for ( bitboard pieces = bb ( move.pc, move.pt ); pieces; )
    {
        /* Get the position of the next piece and unset it in pieces */
        int pos = pieces.trailing_zeros ();
        pieces.reset ( pos );

        /* Check that the position is within the known rank and file */
        if ( known_file != -1 && pos % 8 != known_file ) continue;
        if ( known_rank != -1 && pos / 8 != known_rank ) continue;

        /* Detect if the final position of the move is present in this piece. If so, add it to from_bb. */
        if ( get_move_set ( move.pc, move.pt, pos, check_info ).test ( move.to ) ) from_bb.set ( pos );
    }

    /* If from_bb is empty, throw */
    if ( from_bb.is_empty () ) throw std::runtime_error { "Could not find a matching departure position in fide_deserialize_move ()." };

    /* If from_bb is not a singleton, throw */
    if ( !from_bb.is_singleton () ) throw std::runtime_error { "Could not find a unique departure position in fide_deserialize_move ()." };

    /* Set the departure position */
    move.from = from_bb.trailing_zeros ();



    /* VALIDATION */

    /* Throw if a capture char is not given when required, or given when not required */
    if ( move.capture_pt != ptype::no_piece && !capture_char ) throw std::runtime_error { "Expected a capture character, 'x', in fide_deserialize_move ()." }; 
    if ( move.capture_pt == ptype::no_piece &&  capture_char ) throw std::runtime_error { "Receieved an unexpected capture character, 'x', in fide_deserialize_move ()." }; 

    /* Get if a promotion type is required */
    const bool promote_pt_required = ( move.pt == ptype::pawn && ( move.pc == pcolor::white ? move.to >= 56 : move.to < 8 ) );

    /* Throw if a promotion type is not given when required, or one is given when not required */
    if (  promote_pt_required && move.promote_pt == ptype::no_piece ) throw std::runtime_error { "Expected promotion type (move is a promotion) in fide_deserialize_move ()." };
    if ( !promote_pt_required && move.promote_pt != ptype::no_piece ) throw std::runtime_error { "Unexpected promotion type (move should not promote) in fide_deserialize_move ()." };



    /* CHECK and CHECKMATE */

    /* Create a copy of the bitboard */
    chessboard cb { * this };

    /* Apply the move */
    cb.make_move_internal ( move );

    /* Get whether the other player is in check */
    move.check = cb.is_in_check ( other_color ( move.pc ) );

    /* Get whether the other color has been checkmated */
    move.checkmate = move.check = ( cb.evaluate ( move.pc ) == 10000 );



    /* RETURN */

    /* Return the move */
    return move;
}