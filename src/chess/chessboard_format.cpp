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



/* FORMATTING */

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
            out [ i * 2 ] = piece_chars [ cast_penum ( pt ) ];
            if ( pc == pcolor::black ) out [ i * 2 ] = std::tolower ( out [ i * 2 ] );
        }
    
        /* Possibly add a newline */
        if ( ( i & 7 ) == 7 ) out [ i * 2 + 1 ] = '\n';
    };

    /* Return the formatted string */
    return out;
}



/* MOVE SERIALIZATION */

/** @name  serialize
 * 
 * @brief  Creates a string from the move
 * @return string
 */
std::string chess::chessboard::move_t::serialize () const
{
    /* Check if the move is a castling move */
    if ( is_kingside_castle  () ) return "O-O"; 
    if ( is_queenside_castle () ) return "O-O-O"; 

    /* Create the string for the move */
    std::string out = 
        /* The initial position */
        bitboard::name_cell ( from ) + 
        /* The piece that is moving and a colon */
        piece_chars [ cast_penum ( pt ) ] + ":" +
        /* The final position */
        bitboard::name_cell ( to ) +
        /* The capture type */
        piece_chars [ cast_penum ( capture_pt ) ] +
        /* A '+' for single check or '++' for double check */
        ( check_count == 1 ? "+"  : "" ) +
        ( check_count == 2 ? "++" : "" ) +
        /* A 'ep' if the move is an en passant capture */
        ( en_passant_pos ? "ep" : "" ) +
        /* If promoting a pawn, add a slash and the promotion type */
        ( promote_pt != ptype::no_piece ? std::string ( "/" ) + piece_chars [ cast_penum ( promote_pt ) ] : "" );

    /* Return the string */
    return out;
}

/** @name  deserialize
 * 
 * @brief  Sets the move from a string
 * @param  _pc: The color that is to make the move
 * @param  desc: The serialization of the move
 * @return A reference to this move
 */
chess::chessboard::move_t& chess::chessboard::move_t::deserialize ( const pcolor _pc, const std::string& desc )
{
    /* Set the color */
    pc = _pc;

    /* Look for a castling move */
    if ( desc == "O-O"   ) { pt = ptype::king; capture_pt = ptype::no_piece; promote_pt = ptype::no_piece; from = ( pc == pcolor::white ? 4 : 60 ); to = ( pc == pcolor::white ? 6 : 62 ); check_count = 0; return * this; };
    if ( desc == "O-O-O" ) { pt = ptype::king; capture_pt = ptype::no_piece; promote_pt = ptype::no_piece; from = ( pc == pcolor::white ? 4 : 60 ); to = ( pc == pcolor::white ? 2 : 58 ); check_count = 0; return * this; };

    /* Store the running string position */
    int desc_pos = 0;

    /* Get if the input is too short */
    if ( desc.size () < 7 ) throw std::runtime_error { "Input too short in deserialize_move ()." };



    /* FROM */
    {
        /* Check the cell name is valid */
        if ( desc.at ( desc_pos ) < 'a' || desc.at ( desc_pos ) > 'h' ) throw std::runtime_error { "Invalid from position in deserialize_move ()." };
        if ( desc.at ( desc_pos + 1 ) < '1' || desc.at ( desc_pos + 1 ) > '8' ) throw std::runtime_error { "Invalid from position in deserialize_move ()." };

        /* Extract the cell pos and increase desc_pos */
        from = bitboard::cell_pos ( desc.substr ( desc_pos, 2 ) );
        desc_pos += 2;
    }
    
    /* PT */
    {
        /* Look for the ptype index from the character for pt. Note that the type cannot be any_piece or no_piece, hence the 6. */
        const char * pt_ptr = std::find ( piece_chars, piece_chars + 6, desc.at ( desc_pos ) );

        /* If the character for pt was invalid, throw */
        if ( pt_ptr == piece_chars + 6 ) throw std::runtime_error { "Invalid piece type in deserialize_move ()." };

        /* Set the piece type and increase desc_pos */
        pt = static_cast<ptype> ( pt_ptr - piece_chars );
        desc_pos += 1;
    }

    /* SEPARATING COLON */  
    {
        /* If the colon is not there, throw */
        if ( desc.at ( desc_pos ) != ':' ) throw std::runtime_error { "Invalid format (expected separating ':') in deserialize_move ()." };

        /* Increase desc_pos */
        desc_pos += 1;
    }

    /* TO */
    {
        /* Check the cell name is valid */
        if ( desc.at ( desc_pos ) < 'a' || desc.at ( desc_pos ) > 'h' ) throw std::runtime_error { "Invalid to position in deserialize_move ()." };
        if ( desc.at ( desc_pos + 1 ) < '1' || desc.at ( desc_pos + 1 ) > '8' ) throw std::runtime_error { "Invalid to position in deserialize_move ()." };

        /* Extract the cell pos and increase desc_pos */
        to = bitboard::cell_pos ( desc.substr ( desc_pos, 2 ) );
        desc_pos += 2;
    }

    /* CAPTURE_PT */
    {
        /* Look for the ptype index from the character for capture_pt. Note that the capture type can be no_piece, hence the 8. */
        const char * pt_ptr = std::find ( piece_chars, piece_chars + 8, desc.at ( desc_pos ) );

        /* If the character for capture_pt was invalid, throw */
        if ( pt_ptr == piece_chars + 8 ) throw std::runtime_error { "Invalid capture type in deserialize_move ()." };

        /* Set the piece type and increase desc_pos */
        capture_pt = static_cast<ptype> ( pt_ptr - piece_chars );
        desc_pos += 1;

        /* Check that the capture type is not the king or any_piece */
        if ( capture_pt == ptype::king || capture_pt == ptype::any_piece ) throw std::runtime_error { "Invalid capture type in deserialize_move ()." };
    }

    /* CHECK_COUNT */
    {
        /* Set the check count to zero */
        check_count = 0;

        /* If there are any '+'s, add up the check count and increment desc_pos */
        if ( desc_pos != desc.size () && desc.at ( desc_pos ) == '+' ) { ++check_count; ++desc_pos; }
        if ( desc_pos != desc.size () && desc.at ( desc_pos ) == '+' ) { ++check_count; ++desc_pos; }
    }

    /* EN_PASSANT_POS */
    {
        /* Set the position to 0 */
        en_passant_pos = 0;

        /* Check desc is long enough and the next characters are 'ep' */
        if ( desc_pos + 1 < desc.size () && desc.substr ( desc_pos, 2 ) == "ep" )
        {
            /* Calculate the position and set it */
            en_passant_pos = ( from / 8 ) * 8 + ( to % 8 );

            /* Increase desc_pos */
            desc_pos += 2;
        }
    }

    /* PROMOTE_PT */
    {
        /* Detect if there should be a promotion type */
        if ( pt == ptype::pawn && ( pc == pcolor::white ? to >= 56 : to < 8 ) )
        {
            /* Check desc is long enough to include the promotion type information */
            if ( desc_pos + 1 >= desc.size () ) throw std::runtime_error { "No promotion type in deserialize_move ()." };

            /* Check that the next character is a '/' and increment desc_pos */
            if ( desc.at ( desc_pos ) != '/' ) throw std::runtime_error { "Invalid promotion type in deserialize_move ()." };
            desc_pos += 1;

            /* Look for the ptype index from the character for promote_pt */
            const char * pt_ptr = std::find ( piece_chars, piece_chars + 6, desc.at ( desc_pos ) );

            /* If the character for promote_pt was invalid, throw */
            if ( pt_ptr == piece_chars + 6 ) throw std::runtime_error { "Invalid promotion type in deserialize_move ()." };

            /* Set the piece type and increase desc_pos */
            promote_pt = static_cast<ptype> ( pt_ptr - piece_chars );
            desc_pos += 1;
            
            /* Check that the promotion type is not a pawn or king */
            if ( promote_pt == ptype::pawn || promote_pt == ptype::king ) throw std::runtime_error { "Invalid promotion type in deserialize_move ()." };
        } 
        
        /* Otherwise set promotion type to no_piece */
        else promote_pt == ptype::no_piece;
    }
    
    /* Check if there are any extra characters */
    if ( desc_pos != desc.size () ) throw std::runtime_error { "Input too long in deserialize_move ()." };

    /* Return this */
    return * this;
}