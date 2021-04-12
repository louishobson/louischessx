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
     * pos ^ 56 changes the endianness of pos, such that the top row is read first.
     * Multiplying by 2 skips the spaces inbetween cells.
     */
    std::string out ( 128, ' ' );
    for ( int pos = 0; pos < 64; ++pos ) 
    {
        /* Get the piece color */
        const pcolor pc = find_color ( pos ^ 56 ); 

        /* If there is no piece, output a dot, otherwise find the correct character to output */
        if ( pc == pcolor::no_piece ) out.at ( pos * 2 ) = '.'; else
        {
            /* Find the type */
            const ptype pt = find_type ( pc, pos ^ 56 );
            
            /* Add the right character */
            out.at ( pos * 2 ) = ptype_to_character ( pt );
            if ( pc == pcolor::black ) out.at ( pos * 2 ) = std::tolower ( out.at ( pos * 2 ) );
        }
    
        /* Possibly add a newline */
        if ( ( pos & 7 ) == 7 ) out.at ( pos * 2 + 1 ) = '\n';
    };

    /* Return the formatted string */
    return out;
}



/* FEN BOARD SERIALIZATION */



/** @name  fen_serialize_board
 * 
 * @brief  Serialize the board based on Forsyth–Edwards notation
 * @param  pc: The color who's turn it is next
 * @return string
 */
std::string chess::chessboard::fen_serialize_board ( const pcolor pc ) const
{
    /* Create the output string */
    std::string out;

    /* Iterate for each rank, top to bottom */
    for ( int rank = 7; rank >= 0; --rank ) 
    {
        /* Store the number of consecutive empty cells on this rank */
        int empty = 0;

        /* Iterate through the files */
        for ( int file = 0; file < 8; ++file )
        {
            /* Get the piece color at this position */
            const pcolor this_pc = find_color ( rank, file );

            /* If there is no piece at the position, increment empty */
            if ( this_pc == pcolor::no_piece ) ++empty; else
            {
                /* Else there is a piece. Firstly if empty != 0, output its value and set it to 0. */
                if ( empty ) { out += std::to_string ( empty ); empty = 0; }

                /* Get the type of piece and output the character for this position */
                out += ptype_to_character ( find_type ( this_pc, rank, file ) );

                /* If the color of the piece is back, set the character to lowercase */
                if ( this_pc == pcolor::black ) out.back () = std::tolower ( out.back () );
            }
        }

        /* If empty != 0, output its value */
        if ( empty ) out += std::to_string ( empty );

        /* Output a slash if this is not the last rank, otherwise just a space */
        if ( rank ) out += "/"; else out += " ";
    }

    /* Add a character for who's turn it is next */
    if ( pc == pcolor::white ) out += "w "; else out += "b ";

    /* If neither side has caslting rights, put a dash next */
    if ( !has_any_castling_rights ( pcolor::white ) && !has_any_castling_rights ( pcolor::black ) ) out += "- "; else
    {
        /* Else add the castling rights */
        if ( has_kingside_castling_rights  ( pcolor::white ) ) out += "K";
        if ( has_queenside_castling_rights ( pcolor::white ) ) out += "Q";
        if ( has_kingside_castling_rights  ( pcolor::black ) ) out += "k";
        if ( has_queenside_castling_rights ( pcolor::black ) ) out += "q";

        /* Add a space */
        out += " ";
    }

    /* If there is not an en passant target sqaure, output a dash */
    if ( !aux_info.en_passant_target ) out += "- "; else
    {
        /* Else output the en passant target square */
        out += bitboard::name_cell ( aux_info.en_passant_target ) + " ";
    }

    /* TODO: Implement half-move clock since last capture or pawn push */
    out += "0 ";

    /* Add the fullmove mumber */
    out += std::to_string ( ( game_state_history.size () - 1 ) / 2 + 1 );

    /* Return out */
    return out;
}

/** @name  fen_deserialize_board
 * 
 * @brief  Deserialize a string based on Forsyth–Edwards notation, and replace this board with it.
 *         The game history will be emptied, and this state will be considered the opening state.
 * @param  desc: The board description
 * @return The color who's move it is next
 */
chess::pcolor chess::chessboard::fen_deserialize_board ( const std::string& desc )
{
    /* Create regex for capturing the parts */
    std::regex state_regex { "^((?:[1-8PNBRQKpnbrqk]{1,8}/){7}[1-8PNBRQKpnbrqk]{1,8}) ([wb]) (?:-|(?=[KQkq])(K?)(Q?)(k?)(q?)) (?:-|([a-h][1-8])) ([0-9]+) ([0-9]+)" };

    /* Run the search, throw if no matches were found */
    std::smatch state_match;
    if ( !std::regex_search ( desc, state_match, state_regex ) ) throw chess_input_error { "Could not format board state description in fen_deserialize_board ()." };

    /* Create a new chessboard to decode desc into. Make the chessboard initially empty, but with full castling rights. */
    chessboard cb; cb.reset_to_empty (); cb.aux_info = {};

    /* Iterate through the board string */
    int rank = 7, file = 0;
    for ( const char piece_char : state_match.str ( 1 ) )
    {
        /* Get if the character is a slash */
        if ( piece_char == '/' ) 
        {
            /* If file doesn't equal 8, throw */
            if ( file != 8 ) throw chess_input_error { "Invalid length in board state description in fen_deserialize_board ()." };

            /* Decrement rank, and set file to 0 */
            --rank; file = 0;
        } else

        /* Else get if the character is a number */
        if ( std::isdigit ( piece_char ) )
        {
            /* If file exceeds after adding the number, throw */
            if ( ( file += piece_char - '0' ) > 8 ) throw chess_input_error { "Invalid file length in board state description in fen_deserialize_board ()." };
        } else

        /* Else the character must be a piece type */
        {
            /* Get the color of the piece */
            const pcolor piece_pc = ( std::isupper ( piece_char ) ? pcolor::white : pcolor::black );

            /* Set the correct color at this position */
            cb.get_bb ( piece_pc ).set ( rank, file );

            /* Set the correct piece at this position */
            cb.get_bb ( piece_pc, character_to_ptype ( std::toupper ( piece_char ) ) ).set ( rank, file );

            /* Increment file */
            ++file;
        }
    }

    /* Throw if file != 8 */
    if ( file != 8 ) throw chess_input_error { "Invalid file length in board state description in fen_deserialize_board ()." };

    /* Get the color who's move it is next */
    const pcolor pc = ( state_match.str ( 2 ) == "w" ? pcolor::white : pcolor::black );

    /* Look through the castling rights, and remove them as characters are not present */
    if ( !state_match.length ( 3 ) ) cb.set_kingside_castle_lost  ( pcolor::white );
    if ( !state_match.length ( 4 ) ) cb.set_queenside_castle_lost ( pcolor::white );
    if ( !state_match.length ( 5 ) ) cb.set_kingside_castle_lost  ( pcolor::black );
    if ( !state_match.length ( 6 ) ) cb.set_queenside_castle_lost ( pcolor::black );

    /* Set the en passant target square if given. Assume the color that made the move is opposite to pc */
    if ( state_match.length ( 7 ) ) { cb.aux_info.en_passant_target = bitboard::cell_pos ( state_match.str ( 7 ) ); cb.aux_info.en_passant_color = other_color ( pc ); }
    
    /* Ignore clocks for now */

    /* Set the history of cb */
    cb.game_state_history = { cb.get_game_state ( pcolor::no_piece ) };

    /* Copy over the new chessboard */
    * this = cb;

    /* Return pc */
    return pc;
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
        ( move.promote_pt != ptype::no_piece ? std::string { ptype_to_character ( move.promote_pt ) } : "" ) +
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
chess::move_t chess::chessboard::fide_deserialize_move ( const pcolor pc, const std::string& desc ) const
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

    /* Create the regex to extract the information */
    std::regex move_regex { "^" "([PNBRQK]?)" "([a-h]?)([1-8]?)" "(x?)" "([a-h][1-8])" "[=/]?([NBRQ]?)" };

    /* Run the search, throw if no matches were found */
    std::smatch move_match;
    if ( !std::regex_search ( desc, move_match, move_regex ) ) throw chess_input_error { "Could not format move description in fide_deserialize_move ()." };



    /* EXTRACT INFO */

    /* See if there is a promotion type */
    if ( move_match.length ( 1 ) ) move.pt = character_to_ptype ( move_match.str ( 1 ).at ( 0 ) );
    else move.pt = ptype::pawn;

    /* Get the known file or rank from the disambiguation characters. */
    const int known_file = ( move_match.length ( 2 ) ? move_match.str ( 2 ).at ( 0 ) - 'a' : -1 );
    const int known_rank = ( move_match.length ( 3 ) ? move_match.str ( 3 ).at ( 0 ) - '1' : -1 );

    /* Get if there is a capture character */
    const bool capture_char = move_match.length ( 4 );

    /* Get the destination position */
    move.to = bitboard::cell_pos ( move_match.str ( 5 ) );

    /* Get promotion type, if given */
    if ( move_match.length ( 6 ) ) move.promote_pt = character_to_ptype ( move_match.str ( 6 ).at ( 0 ) );

    /* Determine the capture type from the destination position */
    move.capture_pt = find_type ( other_color ( move.pc ), move.to );

    /* Determine if the move is an en passant capture. If found to be so, change the capture type to pawn. */
    if ( move.pt == ptype::pawn && move.pc == aux_info.en_passant_color && move.to == aux_info.en_passant_target ) move.capture_pt = ptype::pawn;



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
    if ( from_bb.is_empty () ) throw chess_input_error { "Could not find a matching departure position in fide_deserialize_move ()." };

    /* If from_bb is not a singleton, throw */
    if ( !from_bb.is_singleton () ) throw chess_input_error { "Could not find a unique departure position in fide_deserialize_move ()." };

    /* Set the departure position */
    move.from = from_bb.trailing_zeros ();



    /* VALIDATION */

    /* Throw if a capture char is not given when required, or given when not required */
    if ( move.capture_pt != ptype::no_piece && !capture_char ) throw chess_input_error { "Expected a capture character, 'x', in fide_deserialize_move ()." }; 
    if ( move.capture_pt == ptype::no_piece &&  capture_char ) throw chess_input_error { "Receieved an unexpected capture character, 'x', in fide_deserialize_move ()." }; 

    /* Get if a promotion type is required */
    const bool promote_pt_required = ( move.pt == ptype::pawn && ( move.pc == pcolor::white ? move.to >= 56 : move.to < 8 ) );

    /* Throw if a promotion type is not given when required, or one is given when not required */
    if (  promote_pt_required && move.promote_pt == ptype::no_piece ) throw chess_input_error { "Expected promotion type (move is a promotion) in fide_deserialize_move ()." };
    if ( !promote_pt_required && move.promote_pt != ptype::no_piece ) throw chess_input_error { "Unexpected promotion type (move should not promote) in fide_deserialize_move ()." };



    /* CHECK and CHECKMATE */

    /* Create a copy of the bitboard */
    chessboard cb { * this };

    /* Apply the move */
    cb.make_move_internal ( move );

    /* Get whether the other player is in check */
    move.check = cb.is_in_check ( other_color ( move.pc ) );

    /* Get whether the other color has been checkmated */
    move.checkmate = ( cb.evaluate ( move.pc ) == 10000 );



    /* RETURN */

    /* Return the move */
    return move;
}