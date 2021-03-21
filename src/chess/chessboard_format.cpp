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
chess::chessboard::move_t chess::chessboard::fide_deserialize_move ( const pcolor pc, const std::string& desc ) const
{
    /* Look for a castling move */
    if ( desc == "O-O"   ) return move_t { pc, ptype::king, ptype::no_piece,  ptype::no_piece, ( pc == pcolor::white ? 4 : 60 ), ( pc == pcolor::white ? 6 : 62 ) };
    if ( desc == "O-O-O" ) return move_t { pc, ptype::king, ptype::no_piece,  ptype::no_piece, ( pc == pcolor::white ? 4 : 60 ), ( pc == pcolor::white ? 2 : 58 ) };

    /* Throw if the input is too short */
    if ( desc.size () < 2 ) throw std::runtime_error { "Input too short in fide_deserialize_move ()." };

    /* Get the check info */
    const check_info_t check_info = get_check_info ( pc );

    /* Create an empty move */
    move_t move { pc };

    /* Store the running string iterator */
    auto desc_it = desc.begin ();

    /* PT */
    {
        /* See if the string begins with an upper case letter (which gives the piece type). If not present, set type to pawn. */
        if ( std::isupper ( * desc_it ) )
        {
            /* Get the type, also increasing desc_it */
            move.pt = character_to_ptype ( * desc_it++ );

            /* If the character for pt was invalid, throw */
            if ( move.pt == ptype::any_piece || move.pt == ptype::no_piece ) throw std::runtime_error { "Invalid piece type in fide_deserialize_move ()." };
        } else move.pt = ptype::pawn;
    }

    /* Store the submatch to the destination cell name */
    std::ssub_match to_submatch;

    /* TO */
    {
        /* Create the regex */
        std::regex to_regex { "[a-h][1-8]" };

        /* Search for the destination position */
        for ( std::sregex_iterator it { desc.begin (), desc.end (), to_regex }; it != std::sregex_iterator {}; to_submatch = * it++->begin () );
        
        /* Throw if not found */
        if ( !to_submatch.matched ) throw std::runtime_error { "Could not find destination position in fide_deserialize_move ()." };

        /* Extract the cell pos */
        move.to = bitboard::cell_pos ( to_submatch );

        /* Throw if the destination position is a friendly piece */
        if ( bb ( move.pc ).test ( move.to ) ) throw std::runtime_error { "Destination position contains a friendly piece in fide_deserialize_move ()." };
    }

    /* Get the number of disambiguation chars */
    int num_disambiguation_chars = to_submatch.first - desc_it;

    /* CAPTURE_PT and EN_PASSANT_POS */
    {
        /* Determine the capture type from the destination position */
        move.capture_pt = find_type ( other_color ( move.pc ), move.to );

        /* If the move is a pawn attack, the capture type is no_piece, and the pawn ends up behind the last pawn double push, then the move must be en passant.
         * Therefore reassign the capture type to a pawn and set the en passant pos.
         */
        if ( aux_info.double_push_pos && move.pt == ptype::pawn && move.capture_pt == ptype::no_piece && move.to == aux_info.double_push_pos + ( move.pc == pcolor::white ? +8 : -8 ) )
            { move.capture_pt = ptype::pawn; move.en_passant_pos = aux_info.double_push_pos; }

        /* Get if there is a capture character */
        if ( num_disambiguation_chars && * ( to_submatch.first - 1 ) == 'x' )
        { 
            /* There was a capture character, so throw if the move is not a capture */
            if ( move.capture_pt == ptype::no_piece ) throw std::runtime_error { "Receieved an unexpected capture character, 'x', in fide_deserialize_move ()." }; 

            /* Reduce num_disambiguation_chars by one */
            --num_disambiguation_chars;
        } else
        { 
            /* There was no capture character, so throw if the move is a capture */
            if ( move.capture_pt != ptype::no_piece ) throw std::runtime_error { "Expected a capture character, 'x', in fide_deserialize_move ()." }; 
        }
    }

    /* FROM */
    {
        /* If there is more than two disambiguation characters left, throw */
        if ( num_disambiguation_chars > 2 ) throw std::runtime_error { "Too many disambiguation characters in fide_deserialize_move ()." };

        /* Store the given file and rank for the piece */
        int known_file = -1, known_rank = -1;

        /* If there is only one disambiguation, set the known file or rank */
        if ( num_disambiguation_chars == 1 )
        {
            /* Get the file/rank */
            if ( * desc_it >= 'a' && * desc_it <= 'h' ) known_file = * desc_it - 'a'; else
            if ( * desc_it >= '1' && * desc_it <= '8' ) known_rank = * desc_it - '1'; else

            /* Throw since was unknown character */
            throw std::runtime_error { "Invalid departure position character in fide_deserialize_move ()." };
        } else

        /* Else if there are two characters, get the file and rank */
        if ( num_disambiguation_chars == 2 )
        {
            /* Check the characters are valid */
            if ( * (  desc_it    ) < 'a' || * ( desc_it     ) > 'h' ) throw std::runtime_error { "Invalid departure position in fide_deserialize_move ()." };
            if ( * ( desc_it + 1 ) < '1' || * ( desc_it + 1 ) > '8' ) throw std::runtime_error { "Invalid departure position in fide_deserialize_move ()." };

            /* Set the file and rank */
            known_file = * ( desc_it     ) - 'a';
            known_rank = * ( desc_it + 1 ) - '1';
        }

        /* Iterate through the pieces of the moving color and find any that can make the move */
        bitboard from_bb;
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
    }

    /* Set desc_it to after the destination cell name */
    desc_it = to_submatch.second;

    /* PROMOTE_PT */
    {
        /* Get if a promotion type is required */
        const bool promote_pt_required = ( move.pt == ptype::pawn && ( move.pc == pcolor::white ? move.to >= 56 : move.to < 8 ) );

        /* If there is a capital letter at desc_it, get the promotion type */
        if ( desc_it != desc.end () && std::isupper ( * desc_it ) )
        {
            /* Get the type, also increasing desc_it */
            move.promote_pt = character_to_ptype ( * desc_it++ );

            /* If the character for promote_pt was invalid, throw */
            if ( move.promote_pt == ptype::any_piece || move.promote_pt == ptype::no_piece || move.promote_pt == ptype::pawn || move.promote_pt == ptype::king ) throw std::runtime_error { "Invalid promote type in fide_deserialize_move ()." };
        
            /* If a promotion type is not required, throw */
            if ( !promote_pt_required ) throw std::runtime_error { "Unexpected promotion type (move should not promote) in fide_deserialize_move ()." };
        }

        /* If a promotion type is required and not given, throw */
        if ( promote_pt_required ) throw std::runtime_error { "Expected promotion type (move is a promotion) in fide_deserialize_move ()." };
    }

    /* CHECK and CHECKMATE */
    {
        /* Create a copy of the bitboard */
        chessboard cb { * this };

        /* Apply the move */
        cb.make_move_internal ( move );

        /* Get whether the other player is in check */
        move.check = cb.is_in_check ( other_color ( move.pc ) );

        /* Get whether the other color has been checkmated */
        move.checkmate = move.check = ( cb.evaluate ( move.pc ) == 10000 );
    }

    /* VALIDATE CHECK, CHECKMATE and EN_PASSANT */
    {
        /* Look for check '+', and throw if found and check flag is not set */
        if ( desc.find ( '+', desc_it - desc.begin () ) != desc.npos && !move.check )
            throw std::runtime_error { "Move incorrectly labelled as checking in fide_deserialize_move ()." };

        /* Look for checkmate '#', and throw if found and checkmate or check are not set */
        if ( desc.find ( '#', desc_it - desc.begin () ) != desc.npos && ( !move.check || !move.checkmate ) )
            throw std::runtime_error { "Move incorrectly labelled as a checkmate in fide_deserialize_move ()." };
        
        /* Look for en passant string and throw if the move is not en passant */
        if ( desc.find ( "ep", desc_it - desc.begin () ) != desc.npos || desc.find ( "e.p.", desc_it - desc.begin () ) != desc.npos )
           if ( move.en_passant_pos == 0 ) throw std::runtime_error { "Move incorrectly labelled as en passant in fide_deserialize_move ()." };
    }

    /* Return the move */
    return move;
}