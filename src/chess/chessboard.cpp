/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/chessboard.cpp
 * 
 * Implementation of include/chess/chessboard.h
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
    for ( unsigned i = 0; i < 64; ++i ) 
    { 
        const bitboard mask = singleton_bitboard ( i ^ 56 );
        if ( bb () & mask )
        {
            if ( bb ( pcolor::white, ptype::pawn   ) & mask ) out [ i * 2 ] = 'P'; else
            if ( bb ( pcolor::white, ptype::king   ) & mask ) out [ i * 2 ] = 'K'; else
            if ( bb ( pcolor::white, ptype::queen  ) & mask ) out [ i * 2 ] = 'Q'; else
            if ( bb ( pcolor::white, ptype::bishop ) & mask ) out [ i * 2 ] = 'B'; else
            if ( bb ( pcolor::white, ptype::knight ) & mask ) out [ i * 2 ] = 'N'; else
            if ( bb ( pcolor::white, ptype::rook   ) & mask ) out [ i * 2 ] = 'R'; else
            if ( bb ( pcolor::black, ptype::pawn   ) & mask ) out [ i * 2 ] = 'p'; else
            if ( bb ( pcolor::black, ptype::king   ) & mask ) out [ i * 2 ] = 'k'; else
            if ( bb ( pcolor::black, ptype::queen  ) & mask ) out [ i * 2 ] = 'q'; else
            if ( bb ( pcolor::black, ptype::bishop ) & mask ) out [ i * 2 ] = 'b'; else
            if ( bb ( pcolor::black, ptype::knight ) & mask ) out [ i * 2 ] = 'n'; else
            if ( bb ( pcolor::black, ptype::rook   ) & mask ) out [ i * 2 ] = 'r';
        }
        else out [ i * 2 ] = '.';
        if ( ( i & 7 ) == 7 ) out [ i * 2 + 1 ] = '\n';
    };

    /* Return the formatted string */
    return out;
}



/* BOARD EVALUATION */



/** @name  get_check_info
 * 
 * @brief  Get information about the check state of a color's king
 * @param  pc: The color who's king we will look at
 * @return check_info_t
 */
chess::chessboard::check_info_t chess::chessboard::get_check_info ( pcolor pc ) const
{
    /* SETUP */

    /* The output check info */
    check_info_t check_info;

    /* Get the other color */
    const pcolor npc = other_color ( pc );

    /* Get the friendly and opposing pieces */
    const bitboard friendly = bb ( pc );
    const bitboard opposing = bb ( npc );

    /* Get the king and position of the colored king */
    const bitboard king = bb ( pc, ptype::king );
    const unsigned king_pos = king.trailing_zeros_nocheck ();

    /* Get the positions of the opposing straight and diagonal pieces */
    const bitboard op_straight = bb ( npc, ptype::queen ) | bb ( npc, ptype::rook   );
    const bitboard op_diagonal = bb ( npc, ptype::queen ) | bb ( npc, ptype::bishop );

    /* Get the primary propagator.
     * Primary propagator of not opposing means that spans will overlook friendly pieces.
     * The secondary propagator will be universe.
     */
    const bitboard pp = ~opposing;
    const bitboard sp = ~bitboard {};



    /* SLIDING PIECES */

    /* Get the start of the compasses */
    straight_compass straight_dir = straight_compass_start ();
    diagonal_compass diagonal_dir = diagonal_compass_start ();

    /* Iterate through the straight compass to see if those sliding pieces could be attacking */
    if ( bitboard::straight_attack_lookup ( king_pos ) & op_straight )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( straight_dir ), king_pos ) & op_straight )
        {
            /* Span the king in the current direction */
            const bitboard king_span = king.rook_attack ( straight_dir, pp, sp );

            /* Get the checking and blocking pieces */
            const bitboard checking = king_span & op_straight;
            const bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.check_vectors |= king_span.only_if ( checking.is_nonempty () & blocking.is_empty () );
            check_info.block_vectors |= king_span.only_if ( checking.is_nonempty () & blocking.is_singleton () );
        }

        /* Increment compass */
        straight_dir = compass_next ( straight_dir );
    }

    /* Iterate through the diagonal compass to see if those sliding pieces could be attacking */
    if ( bitboard::diagonal_attack_lookup ( king_pos ) & op_diagonal )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( diagonal_dir ), king_pos ) & op_diagonal ) 
        {
            /* Span the king in the current direction */
            const bitboard king_span = king.bishop_attack ( diagonal_dir, pp, sp );

            /* Get the checking and blocking pieces */
            const bitboard checking = king_span & op_diagonal;
            const bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.check_vectors |= king_span.only_if ( checking.is_nonempty () & blocking.is_empty () );
            check_info.block_vectors |= king_span.only_if ( checking.is_nonempty () & blocking.is_singleton () );
        }
        
        /* Increment compass */
        diagonal_dir = compass_next ( diagonal_dir );
    }



    /* KING, KNIGHTS AND PAWNS */

    /* Throw if kings are adjacent (this should never occur) */
    if ( bitboard::king_attack_lookup ( king_pos ) & bb ( npc, ptype::king ) ) [[ unlikely ]] throw std::runtime_error { "Adjacent king found in check_info ()." };

    /* Add checking knights */
    check_info.check_vectors |= bitboard::knight_attack_lookup ( king_pos ) & bb ( npc, ptype::knight );

    /* Switch depending on pc and add checking pawns */
    if ( pc == pcolor::white )
        check_info.check_vectors |= king.pawn_any_attack_n () & bb ( pcolor::black, ptype::pawn );
    else
        check_info.check_vectors |= king.pawn_any_attack_s () & bb ( pcolor::white, ptype::pawn );



    /* Return the info */
    return check_info;    
}



/** @name  is_protected
 * 
 * @brief  Returns true if the board position is protected by the player specified.
 *         There is no restriction on what piece is at the position, since any piece in the position is ignored.
 * @param  pc: The color who is defending.
 * @param  pos: The position of the cell to check the defence of.
 * @return boolean
 */
bool chess::chessboard::is_protected ( pcolor pc, unsigned pos ) const noexcept
{
    /* SETUP */

    /* Get a bitboard from pos */
    const bitboard pos_bb = singleton_bitboard ( pos );

    /* Get the positions of the friendly straight and diagonal pieces */
    const bitboard fr_straight = bb ( pc, ptype::queen ) | bb ( pc, ptype::rook );
    const bitboard fr_diagonal = bb ( pc, ptype::queen ) | bb ( pc, ptype::bishop );

    /* Get the primary propagator.
     * Primary propagator of not non-occupied will mean the span will stop at the first piece.
     * The secondary propagator of friendly pieces means the span will include a friendly piece if found.
     */
    const bitboard pp = ~bb ();
    const bitboard sp = bb ( pc );

    /* Get the adjacent open cells. These are the cells which don't contain an enemy piece or a friendly pawn or knight (and protection coule be given by a sliding piece) */
    const bitboard adj_open_cells = bitboard::king_attack_lookup ( pos ) & ~bb ( other_color ( pc ) ) & ~bb ( pc, ptype::pawn ) & ~bb ( pc, ptype::knight );



    /* KING, KNIGHTS AND PAWNS */

    /* Look for an adjacent king */
    if ( bitboard::king_attack_lookup ( pos ) & bb ( pc, ptype::king ) ) return true;

    /* Look for defending knights */
    if ( bitboard::knight_attack_lookup ( pos ) & bb ( pc, ptype::knight ) ) return true;

    /* Switch depending on pc and look for defening pawns */
    if ( pc == pcolor::white )
        { if ( pos_bb.pawn_any_attack_s () & bb ( pcolor::white, ptype::pawn ) ) return true; }
    else
        { if ( pos_bb.pawn_any_attack_n () & bb ( pcolor::black, ptype::pawn ) ) return true; }



    /* SLIDING PIECES */

    /* Get the start of the compasses */
    straight_compass straight_dir = straight_compass_start ();
    diagonal_compass diagonal_dir = diagonal_compass_start ();

    /* Iterate through the straight compass to see if those sliding pieces could be defending */
    if ( bitboard::straight_attack_lookup ( pos ).contains_any ( adj_open_cells ) & bitboard::straight_attack_lookup ( pos ).contains_any ( fr_straight ) )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( straight_dir ), pos ).contains_any ( adj_open_cells ) & bitboard::omnidir_attack_lookup ( static_cast<compass> ( straight_dir ), pos ).contains_any ( fr_straight ) )
            if ( pos_bb.rook_attack ( straight_dir, pp, sp ) & fr_straight ) return true;      

        /* Increment compass */
        straight_dir = compass_next ( straight_dir );
    }

    /* Iterate through the diagonal compass to see if those sliding pieces could be defending */
    if ( bitboard::diagonal_attack_lookup ( pos ).contains_any ( adj_open_cells ) & bitboard::diagonal_attack_lookup ( pos ).contains_any ( fr_diagonal ) )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected  */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( diagonal_dir ), pos ).contains_any ( adj_open_cells ) & bitboard::omnidir_attack_lookup ( static_cast<compass> ( diagonal_dir ), pos ).contains_any ( fr_diagonal ) )
            if ( pos_bb.bishop_attack ( diagonal_dir, pp, sp ) & fr_diagonal ) return true;

        /* Increment compass */
        diagonal_dir = compass_next ( diagonal_dir );
    }



    /* Return false */
    return false;    
}




/** @name  evaluate
 * 
 * @brief  Symmetrically evaluate the board state.
 *         Note that although is non-const, a call to this function will leave the board unmodified.
 * @param  pc: The color who's move it is next
 * @return Integer value, positive for pc, negative for not pc
 */
int chess::chessboard::evaluate ( pcolor pc )
{
    /* TERMINOLOGY */

    /* Here, a piece refers to any chessman, including pawns and kings.
     * Restricted pieces, or restrictives, do not include pawns or kings. 
     *
     * It's important to note the difference between general attacks and legal attacks.
     *
     * An attack is any position a piece could end up in and potentially capture, regardless of if A) it leaves the king in check, or B) if a friendly piece is in the attack set.
     * Legal attacks must be currently possible moves, including A) not leaving the king in check, and B) not attacking friendly pieces, and C) pawns not attacking empty cells.
     * Note that a pawn push is not an attack, since a pawn cannot capture by pushing.
     * 
     * A capture is a legal attack on an enemy piece.
     * A restricted capture is a legal attack on a restricted enemy piece.
     * 
     * Mobility is the number of distinct strictly legal moves. If a mobility is zero, that means that color is in checkmate.
     */



    /* CONSTANTS */

    /* Masks */
    constexpr bitboard white_center { 0x0000181818000000 }, black_center { 0x0000001818180000 };
    constexpr bitboard white_bishop_initial_cells { 0x0000000000000024 }, black_bishop_initial_cells { 0x2400000000000000 };
    constexpr bitboard white_knight_initial_cells { 0x0000000000000042 }, black_knight_initial_cells { 0x4200000000000000 };

    /* Material values */
    constexpr int QUEEN  { 19 };
    constexpr int ROOK   { 10 };
    constexpr int BISHOP {  7 };
    constexpr int KNIGHT {  7 };
    constexpr int PAWN   {  2 };

    /* Pawns */
    constexpr int PAWN_GENERAL_ATTACKS             {   1 }; // For every generally attacked cell
    constexpr int CENTER_PAWNS                     {  20 }; // For every pawn
    constexpr int PAWN_CENTER_GENERAL_ATTACKS      {  10 }; // For every generally attacked cell (in the center)
    constexpr int ISOLATED_PAWNS                   { -10 }; // For every pawn
    constexpr int ISOLATED_PAWNS_ON_SEMIOPEN_FILES { -10 }; // For every pawn
    constexpr int DOUBLED_PAWNS                    {  -5 }; // Tripled pawns counts as -10 etc.
    constexpr int PAWN_GENERAL_ATTACKS_ADJ_OP_KING {  20 }; // For every generally attacked cell
    constexpr int PHALANGA                         {  20 }; // Tripple pawn counts as 40 etc.
    constexpr int BLOCKED_PASSED_PAWNS             { -15 }; // For each blocked passed pawn

    /* Sliding pieces */
    constexpr int STRAIGHT_PIECES_ON_7TH_RANK                      { 30 }; // For each piece
    constexpr int DOUBLE_BISHOP                                    { 20 }; // If true
    constexpr int STRAIGHT_PIECES_ON_OPEN_FILE                     { 35 }; // For each piece
    constexpr int STRAIGHT_PIECES_ON_SEMIOPEN_FILE                 { 25 }; // For each piece
    constexpr int STRAIGHT_PIECE_LEGAL_ATTACKS_ON_OPEN_FILES       { 10 }; // For every legal attack
    constexpr int STRAIGHT_PIECE_LEGAL_ATTACKS_ON_SEMIOPEN_FILES   {  5 }; // For every legal attack
    constexpr int STRAIGHT_PIECES_BEHIND_PASSED_PAWNS              { 20 }; // For every piece
    constexpr int DIAGONAL_PIECE_RESTRICTED_CAPTURES               { 15 }; // For every restricted capture on enemy pieces (not including pawns or kings)
    constexpr int RESTRICTIVES_LEGALLY_ATTACKED_BY_DIAGONAL_PIECES { 15 }; // For every restrictive, white and black (pieces not including pawns or kings)

    /* Knights */
    constexpr int CENTER_KNIGHTS { 20 }; // For every knight

    /* Bishops and knights */
    constexpr int BISHOP_OR_KNIGHT_INITIAL_CELL                 { -15 }; // For every bishop/knight
    constexpr int DIAGONAL_OR_KNIGHT_CAPTURE_ON_STRAIGHT_PIECES {  10 }; // For every capture

    /* Mobility and king queen mobility, for every move */
    constexpr int MOBILITY            {  1 }; // For every legal move
    constexpr int KING_QUEEN_MOBILITY { -2 }; // For every non-capture attack, pretending the king is a queen

    /* Casling */
    constexpr int CASTLE_MADE {  30 }; // If true
    constexpr int CASTLE_LOST { -60 }; // If true

    /* Other values */
    constexpr int KNIGHT_AND_QUEEN_EXIST               { 10 }; // If true
    constexpr int CENTER_LEGAL_ATTACKS_BY_RESTRICTIVES { 10 }; // For every attack (not including pawns or kings)

    /* Checkmate */
    constexpr int CHECKMATE { 5000 }; // If true


    
    /* SETUP */

    /* Get the check info */
    const check_info_t white_check_info = get_check_info ( pcolor::white );
    const check_info_t black_check_info = get_check_info ( pcolor::black );
    const bitboard white_check_vectors = white_check_info.check_vectors, black_check_vectors = black_check_info.check_vectors;
    const bitboard white_block_vectors = white_check_info.block_vectors, black_block_vectors = black_check_info.block_vectors;

    /* Get the check count */
    const unsigned white_check_count = ( white_check_vectors & bb ( pcolor::black ) ).popcount ();
    const unsigned black_check_count = ( black_check_vectors & bb ( pcolor::white ) ).popcount ();

    /* Throw if opposing color is in check */
    if ( ( pc == pcolor::white ? black_check_count : white_check_count ) != 0 ) [[ unlikely ]] throw std::runtime_error { "Opposing color is in check in evaluate ()." };
    
    /* Get king positions */
    const unsigned white_king_pos = bb ( pcolor::white, ptype::king ).trailing_zeros_nocheck ();
    const unsigned black_king_pos = bb ( pcolor::black, ptype::king ).trailing_zeros_nocheck ();

    /* Get the king spans */
    const bitboard white_king_span = bitboard::king_attack_lookup ( white_king_pos );
    const bitboard black_king_span = bitboard::king_attack_lookup ( black_king_pos );

    /* Straight and diagonal block vectors */
    const bitboard white_straight_block_vectors = white_block_vectors & bitboard::straight_attack_lookup ( white_king_pos );
    const bitboard white_diagonal_block_vectors = white_block_vectors & bitboard::diagonal_attack_lookup ( white_king_pos );
    const bitboard black_straight_block_vectors = black_block_vectors & bitboard::straight_attack_lookup ( black_king_pos );
    const bitboard black_diagonal_block_vectors = black_block_vectors & bitboard::diagonal_attack_lookup ( black_king_pos );

    /* Set check_vectors_dep_check_count.
     *
     * This can be intersected with possible attacks to ensure that the king was protected.
     * If not in check, the king does not need to be protected so the bitboard is set to universe.
     * If in check once, the check vecctors must be blocked so the bitboard is set to check_vectors.
     * If in double check, only the king can move so the bitboard is set to empty.
     */
    const bitboard white_check_vectors_dep_check_count = white_check_vectors.all_if ( white_check_count == 0 ).only_if ( white_check_count < 2 );
    const bitboard black_check_vectors_dep_check_count = black_check_vectors.all_if ( black_check_count == 0 ).only_if ( black_check_count < 2 );
    
    /* Set legalize_attacks. This is the intersection of not friendly pieces, not the enemy king, and check_vectors_dep_check_count. */
    const bitboard white_legalize_attacks = ~bb ( pcolor::white ) & ~bb ( pcolor::black, ptype::king ) & white_check_vectors_dep_check_count;
    const bitboard black_legalize_attacks = ~bb ( pcolor::black ) & ~bb ( pcolor::white, ptype::king ) & black_check_vectors_dep_check_count;

    /* Get the restrictives */
    const bitboard white_restrictives = bb ( pcolor::white, ptype::bishop ) | bb ( pcolor::white, ptype::rook ) | bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::knight );
    const bitboard black_restrictives = bb ( pcolor::black, ptype::bishop ) | bb ( pcolor::black, ptype::rook ) | bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::knight );
    const bitboard restrictives = white_restrictives | black_restrictives;

    /* Set the primary propagator such that all empty cells are set */
    const bitboard pp = ~bb ();

    /* Get the white pawn rear span */
    const bitboard white_pawn_rear_span = bb ( pcolor::white, ptype::pawn ).span ( compass::s );
    const bitboard black_pawn_rear_span = bb ( pcolor::black, ptype::pawn ).span ( compass::n );

    /* Get the pawn file fills (from the rear span and forwards fill) */
    const bitboard white_pawn_file_fill = white_pawn_rear_span | bb ( pcolor::white, ptype::pawn ).fill ( compass::n );
    const bitboard black_pawn_file_fill = black_pawn_rear_span | bb ( pcolor::black, ptype::pawn ).fill ( compass::s );

    /* Get the open and semiopen files. White semiopen files contain no white pawns. */
    const bitboard open_files = ~( white_pawn_file_fill | black_pawn_file_fill );
    const bitboard white_semiopen_files = ~white_pawn_file_fill & black_pawn_file_fill;
    const bitboard black_semiopen_files = ~black_pawn_file_fill & white_pawn_file_fill;

    /* Get the cells behind passed pawns. 
     * This is the span between the passed pawns and the next piece back, including that piece.
     */ 
    const bitboard white_behind_passed_pawns = ( bb ( pcolor::white, ptype::pawn ) & ~white_pawn_rear_span ).span ( compass::s, pp, ~bitboard {} ) & black_semiopen_files;
    const bitboard black_behind_passed_pawns = ( bb ( pcolor::black, ptype::pawn ) & ~black_pawn_rear_span ).span ( compass::n, pp, ~bitboard {} ) & white_semiopen_files;



    /* ACCUMULATORS */

    /* The value of the evaluation function */
    int value = 0;

    /* Mobilities */
    int white_mobility = 0, black_mobility = 0;

    /* Partial defence unions. Gives cells which are protected by friendly pieces.
     * This is found from the union of all general attacks and can be used to determine opposing king's possible moves.
     * However, blocking sliding pieces' general attacks are not included in this union (due to complexity to calculate), hence 'partial'.
     */
    bitboard white_partial_defence_union, black_partial_defence_union;

    /* Accumulate the difference between white and black's straight piece legal attacks on open and semiopen files */
    int straight_legal_attacks_open_diff = 0, straight_legal_attacks_semiopen_diff = 0;

    /* Accumulate the difference between white and black's attacks the center by restrictives */
    int center_legal_attacks_by_restrictives_diff = 0;

    /* Accumulate the difference between white and black's number of restricted captures by diagonal pieces */
    int diagonal_restricted_captures_diff = 0;

    /* Accumulate the restricted pieces attacked by diagonal pieces */
    bitboard restrictives_legally_attacked_by_white_diagonal_pieces, restrictives_legally_attacked_by_black_diagonal_pieces;

    /* Accumulate bishop or knight captures on enemy straight pieces */
    int diagonal_or_knight_captures_on_straight_diff = 0;



    /* NON-BLOCKING SLIDING PIECES */

    /* Deal with non-blocking sliding pieces of both colors in one go.
     * If in double check, check_vectors_dep_check_count will remove all attacks the attacks.
     */ 

    /* Scope new bitboards */
    {        
        /* Set straight and diagonal pieces */
        const bitboard white_straight_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook   ) ) & ~white_block_vectors;
        const bitboard white_diagonal_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::bishop ) ) & ~white_block_vectors;
        const bitboard black_straight_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook   ) ) & ~black_block_vectors;
        const bitboard black_diagonal_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::bishop ) ) & ~black_block_vectors;

        /* Start compasses */
        straight_compass straight_dir = straight_compass_start ();
        diagonal_compass diagonal_dir = diagonal_compass_start ();

        /* Iterate through the compass to get all queen, rook and bishop attacks */
        #pragma clang loop unroll ( full )
        #pragma GCC unroll 4
        for ( unsigned i = 0; i < 4; ++i )
        {
            /* Apply all shifts to get straight and diagonal general attacks (note that sp is universe to get general attacks)
             * This allows us to union to the defence set first.
             */
            bitboard white_straight_attacks = white_straight_pieces.rook_attack   ( straight_dir, pp, ~bitboard {} );
            bitboard white_diagonal_attacks = white_diagonal_pieces.bishop_attack ( diagonal_dir, pp, ~bitboard {} );
            bitboard black_straight_attacks = black_straight_pieces.rook_attack   ( straight_dir, pp, ~bitboard {} );
            bitboard black_diagonal_attacks = black_diagonal_pieces.bishop_attack ( diagonal_dir, pp, ~bitboard {} );

            /* Union defence */
            white_partial_defence_union |= white_straight_attacks | white_diagonal_attacks;
            black_partial_defence_union |= black_straight_attacks | black_diagonal_attacks;

            /* Legalize the attacks */
            white_straight_attacks &= white_legalize_attacks;
            white_diagonal_attacks &= white_legalize_attacks;
            black_straight_attacks &= black_legalize_attacks;
            black_diagonal_attacks &= black_legalize_attacks;

            /* Sum mobility */
            white_mobility += white_straight_attacks.popcount () + white_diagonal_attacks.popcount ();
            black_mobility += black_straight_attacks.popcount () + black_diagonal_attacks.popcount ();

            /* Sum rook attacks on open files */
            straight_legal_attacks_open_diff += ( white_straight_attacks & open_files ).popcount ();
            straight_legal_attacks_open_diff -= ( black_straight_attacks & open_files ).popcount ();

            /* Sum rook attacks on semiopen files */
            straight_legal_attacks_semiopen_diff += ( white_straight_attacks & white_semiopen_files ).popcount ();
            straight_legal_attacks_semiopen_diff -= ( black_straight_attacks & black_semiopen_files ).popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff += ( white_straight_attacks & white_center ).popcount () + ( white_diagonal_attacks & white_center ).popcount ();
            center_legal_attacks_by_restrictives_diff -= ( black_straight_attacks & black_center ).popcount () + ( black_diagonal_attacks & black_center ).popcount ();

            /* Union restrictives attacked by diagonal pieces */
            restrictives_legally_attacked_by_white_diagonal_pieces |= restrictives & white_diagonal_attacks;
            restrictives_legally_attacked_by_black_diagonal_pieces |= restrictives & black_diagonal_attacks;

            /* Sum the restricted captures by diagonal pieces */
            diagonal_restricted_captures_diff += ( white_diagonal_attacks & black_restrictives ).popcount ();
            diagonal_restricted_captures_diff -= ( black_diagonal_attacks & white_restrictives ).popcount ();

            /* Sum the legal captures on enemy straight pieces by diagonal pieces */
            diagonal_or_knight_captures_on_straight_diff += ( white_diagonal_attacks & ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook ) ) ).popcount ();
            diagonal_or_knight_captures_on_straight_diff -= ( black_diagonal_attacks & ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) ) ).popcount ();

            /* Increment compasses */
            straight_dir = compass_next ( straight_dir );
            diagonal_dir = compass_next ( diagonal_dir );
        }
    }



    /* BLOCKING SLIDING PIECES */

    /* Blocking pieces are handelled separately for each color.
     * Only if that color is not in check will they be considered.
     * The flood spans must be calculated separately for straight and diagonal, since otherwise the flood could spill onto adjacent block vectors.
     */

    /* White blocking pieces */
    if ( white_check_count == 0 )
    {
        /* Get the straight and diagonal blocking pieces which can move.
         * Hence the straight blocking pieces are all on straight block vectors, and diagonal blocking pieces are all on diagonal block vectors.
         */
        const bitboard straight_blocking_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook   ) ) & white_straight_block_vectors;
        const bitboard diagonal_blocking_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::bishop ) ) & white_diagonal_block_vectors;

        /* Initially set blocking attacks to empty */
        bitboard straight_blocking_attacks, diagonal_blocking_attacks;

        /* Flood span straight and diagonal (but only if there are pieces to flood) */
        if ( straight_blocking_pieces ) straight_blocking_attacks = straight_blocking_pieces.straight_flood_span ( white_straight_block_vectors );
        if ( diagonal_blocking_pieces ) diagonal_blocking_attacks = diagonal_blocking_pieces.diagonal_flood_span ( white_diagonal_block_vectors );

        /* Get the union of blocking attacks */
        const bitboard blocking_attacks = straight_blocking_attacks | diagonal_blocking_attacks;

        /* Only continue if there were appropriate blocking attacks */
        if ( blocking_attacks )
        {
            /* Sum rook attacks on open and semiopen files */
            straight_legal_attacks_open_diff += ( straight_blocking_attacks & open_files ).popcount ();
            straight_legal_attacks_semiopen_diff += ( straight_blocking_attacks & white_semiopen_files ).popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff += ( blocking_attacks & white_center ).popcount ();

            /* Union restrictives attacked by diagonal pieces */
            restrictives_legally_attacked_by_white_diagonal_pieces |= restrictives & diagonal_blocking_attacks;

            /* Get the number of diagonal restricted captures */
            diagonal_restricted_captures_diff += ( diagonal_blocking_attacks & black_restrictives ).popcount ();

            /* Sum the legal captures on enemy straight pieces by diagonal pieces */
            diagonal_or_knight_captures_on_straight_diff += ( diagonal_blocking_attacks & ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook ) ) ).popcount ();

            /* Union defence (at this point the defence union becomes partial) */
            white_partial_defence_union |= blocking_attacks | bb ( pcolor::white, ptype::king );

            /* Sum mobility */
            white_mobility += blocking_attacks.popcount ();
        }
    }

    /* Black blocking pieces */
    if ( black_check_count == 0 )
    {
        /* Get the straight and diagonal blocking pieces which can move.
         * Hence the straight blocking pieces are all on straight block vectors, and diagonal blocking pieces are all on diagonal block vectors.
         */
        const bitboard straight_blocking_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook   ) ) & black_straight_block_vectors;
        const bitboard diagonal_blocking_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::bishop ) ) & black_diagonal_block_vectors;

        /* Initially set blocking attacks to empty */
        bitboard straight_blocking_attacks, diagonal_blocking_attacks;

        /* Flood span straight and diagonal (but only if there are pieces to flood) */
        if ( straight_blocking_pieces ) straight_blocking_attacks = straight_blocking_pieces.straight_flood_span ( black_straight_block_vectors );
        if ( diagonal_blocking_pieces ) diagonal_blocking_attacks = diagonal_blocking_pieces.diagonal_flood_span ( black_diagonal_block_vectors );

        /* Get the union of blocking attacks */
        const bitboard blocking_attacks = straight_blocking_attacks | diagonal_blocking_attacks;

        /* Only continue if there were appropriate blocking attacks */
        if ( blocking_attacks )
        {
            /* Sum rook attacks on open and semiopen files */
            straight_legal_attacks_open_diff -= ( straight_blocking_attacks & open_files ).popcount ();
            straight_legal_attacks_semiopen_diff -= ( straight_blocking_attacks & black_semiopen_files ).popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff -= ( blocking_attacks & black_center ).popcount ();

            /* Union restrictives attacked by diagonal pieces */
            restrictives_legally_attacked_by_black_diagonal_pieces |= restrictives & diagonal_blocking_attacks;

            /* Get the number of diagonal restricted captures */
            diagonal_restricted_captures_diff -= ( diagonal_blocking_attacks & white_restrictives ).popcount ();

            /* Sum the legal captures on enemy straight pieces by diagonal pieces */
            diagonal_or_knight_captures_on_straight_diff -= ( diagonal_blocking_attacks & ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) ) ).popcount ();

            /* Union defence (at this point the defence union becomes partial) */
            black_partial_defence_union |= blocking_attacks | bb ( pcolor::black, ptype::king );

            /* Sum mobility */
            black_mobility += blocking_attacks.popcount ();
        }
    }



    /* GENERAL SLIDING PIECES */

    /* Scope new bitboards */
    {
        /* Get straight pieces */
        const bitboard white_straight_pieces = bb ( pcolor::white, ptype::rook ) | bb ( pcolor::white, ptype::queen );
        const bitboard black_straight_pieces = bb ( pcolor::black, ptype::rook ) | bb ( pcolor::black, ptype::queen );



        /* Incorperate the material cost into value */
        value += BISHOP * ( bb ( pcolor::white, ptype::bishop ).popcount () - bb ( pcolor::black, ptype::bishop ).popcount () );
        value += ROOK   * ( bb ( pcolor::white, ptype::rook   ).popcount () - bb ( pcolor::black, ptype::rook   ).popcount () );
        value += QUEEN  * ( bb ( pcolor::white, ptype::queen  ).popcount () - bb ( pcolor::black, ptype::queen  ).popcount () );

        /* Incorporate straight pieces on 7th (or 2nd) rank into value */
        {
            const bitboard white_straight_pieces_7th_rank = white_straight_pieces & bitboard { bitboard::masks::rank_7 };
            const bitboard black_straight_pieces_2nd_rank = black_straight_pieces & bitboard { bitboard::masks::rank_2 };
            value += STRAIGHT_PIECES_ON_7TH_RANK * ( white_straight_pieces_7th_rank.popcount () - black_straight_pieces_2nd_rank.popcount () );
        }

        /* Incorporate bishops on initial cells into value */
        {
            const bitboard white_initial_bishops = bb ( pcolor::white, ptype::bishop ) & white_bishop_initial_cells;
            const bitboard black_initial_bishops = bb ( pcolor::black, ptype::bishop ) & black_bishop_initial_cells;
            value += BISHOP_OR_KNIGHT_INITIAL_CELL * ( white_initial_bishops.popcount () - black_initial_bishops.popcount () );
        }

        /* Incorporate double bishops into value */
        value += DOUBLE_BISHOP * ( ( bb ( pcolor::white, ptype::bishop ).popcount () == 2 ) - ( bb ( pcolor::black, ptype::bishop ).popcount () == 2 ) );

        /* Incorporate straight pieces and attacks on open/semiopen files into value */
        value += STRAIGHT_PIECES_ON_OPEN_FILE     * ( ( white_straight_pieces & open_files ).popcount () - ( black_straight_pieces & open_files ).popcount () );
        value += STRAIGHT_PIECES_ON_SEMIOPEN_FILE * ( ( white_straight_pieces & white_semiopen_files ).popcount () - ( black_straight_pieces & black_semiopen_files ).popcount () );
        value += STRAIGHT_PIECE_LEGAL_ATTACKS_ON_OPEN_FILES     * straight_legal_attacks_open_diff;
        value += STRAIGHT_PIECE_LEGAL_ATTACKS_ON_SEMIOPEN_FILES * straight_legal_attacks_semiopen_diff;

        /* Incorporate straight pieces behind passed pawns into value */
        value += STRAIGHT_PIECES_BEHIND_PASSED_PAWNS * ( ( white_straight_pieces & white_behind_passed_pawns ).popcount () - ( black_straight_pieces & black_behind_passed_pawns ).popcount () );

        /* Incorporate restrictives attacked by diagonal pieces */
        value += RESTRICTIVES_LEGALLY_ATTACKED_BY_DIAGONAL_PIECES * ( restrictives_legally_attacked_by_white_diagonal_pieces.popcount () - restrictives_legally_attacked_by_black_diagonal_pieces.popcount () );

        /* Incorporate diagonal pieces restricted captures into value */
        value += DIAGONAL_PIECE_RESTRICTED_CAPTURES * diagonal_restricted_captures_diff;
    }



    /* KNIGHTS */

    /* Each color is considered separately in loops */

    /* Scope new bitboards */
    {
        /* Get the knights as temporary variables */
        bitboard white_knights = bb ( pcolor::white, ptype::knight );
        bitboard black_knights = bb ( pcolor::black, ptype::knight );

        /* Iterate through white knights to get attacks.
         * If in double check, white_check_vectors_dep_check_count will remove all moves. 
         */
        while ( white_knights )
        {
            /* Get the position of the next knight and its general attacks */
            const unsigned pos = white_knights.trailing_zeros_nocheck ();
            bitboard knight_attacks = bitboard::knight_attack_lookup ( pos );

            /* Union defence */
            white_partial_defence_union |= knight_attacks;

            /* Find legal knight attacks. Note that a blocking knight cannot move along its block vector, hence cannot move at all. */
            knight_attacks = knight_attacks.only_if ( !white_block_vectors.test ( pos ) ) & white_legalize_attacks;

            /* Sum mobility */
            white_mobility += knight_attacks.popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff += ( knight_attacks & white_center ).popcount ();

            /* Sum the legal captures on enemy straight pieces by knights */
            diagonal_or_knight_captures_on_straight_diff += ( knight_attacks & ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook ) ) ).popcount ();

            /* Unset this bit */
            white_knights.reset ( pos );
        }

        /* Iterate through black knights to get attacks */
        while ( black_knights )
        {
            /* Get the position of the next knight and its general attacks */
            const unsigned pos = black_knights.trailing_zeros_nocheck ();
            bitboard knight_attacks = bitboard::knight_attack_lookup ( pos );

            /* Union defence */
            black_partial_defence_union |= knight_attacks;

            /* Find legal knight attacks. Note that a blocking knight cannot move along its block vector, hence cannot move at all. */
            knight_attacks = knight_attacks.only_if ( !black_block_vectors.test ( pos ) ) & black_legalize_attacks;

            /* Sum mobility */
            black_mobility += knight_attacks.popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff -= ( knight_attacks & black_center ).popcount ();

            /* Sum the legal captures on enemy straight pieces by knights */
            diagonal_or_knight_captures_on_straight_diff -= ( knight_attacks & ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) ) ).popcount ();

            /* Unset this bit */
            black_knights.reset ( pos );
        }



        /* Incorperate the number of knights into value */
        value += KNIGHT * ( bb ( pcolor::white, ptype::knight ).popcount () - bb ( pcolor::black, ptype::knight ).popcount () );

        /* Incorporate knights on initial cells into value */
        {
            const bitboard white_initial_knights = bb ( pcolor::white, ptype::knight ) & white_knight_initial_cells;
            const bitboard black_initial_knights = bb ( pcolor::black, ptype::knight ) & black_knight_initial_cells;
            value += BISHOP_OR_KNIGHT_INITIAL_CELL * ( white_initial_knights.popcount () - black_initial_knights.popcount () );
        }

        /* Incorporate knights in the venter into value */
        {
            const bitboard white_center_knights = bb ( pcolor::white, ptype::knight ) & white_center;
            const bitboard black_center_knights = bb ( pcolor::black, ptype::knight ) & black_center;
            value += CENTER_KNIGHTS * ( white_center_knights.popcount () - black_center_knights.popcount () );
        }
    }



    /* PAWN MOVES */

    /* We can handle both colors, blocking and non-blocking pawns simultaneously, since pawn moves are more simple than sliding pieces and knights.
     * Non-blocking pawns can have their moves calculated normally, but blocking pawns must first be checked as to whether they will move along their blocking vector.
     * There is no if-statement for double check, only check_vectors_dep_check_count is used to mask out all moves if in double check.
     * This is because double check is quite unlikely, and pawn move calculations are quite cheap anyway.
     */

    /* Scope the new bitboards */
    {
        /* Get the non-blocking pawns */
        const bitboard white_non_blocking_pawns = bb ( pcolor::white, ptype::pawn ) & ~white_block_vectors;
        const bitboard black_non_blocking_pawns = bb ( pcolor::black, ptype::pawn ) & ~black_block_vectors;
        
        /* Get the straight and diagonal blocking pawns */
        const bitboard white_straight_blocking_pawns = bb ( pcolor::white, ptype::pawn ) & white_straight_block_vectors;
        const bitboard white_diagonal_blocking_pawns = bb ( pcolor::white, ptype::pawn ) & white_diagonal_block_vectors;
        const bitboard black_straight_blocking_pawns = bb ( pcolor::black, ptype::pawn ) & black_straight_block_vectors;
        const bitboard black_diagonal_blocking_pawns = bb ( pcolor::black, ptype::pawn ) & black_diagonal_block_vectors;

        /* Calculations for pawn pushes.
         * Non-blocking pawns can push without restriction.
         * Blocking pawns can push only if they started and ended within a straight block vector.
         * If in check, ensure that the movements protected the king.
         */
        const bitboard white_pawn_pushes = ( white_non_blocking_pawns.pawn_push_n ( pp ) | ( white_straight_blocking_pawns.pawn_push_n ( pp ) & white_straight_block_vectors ) ) & white_check_vectors_dep_check_count;
        const bitboard black_pawn_pushes = ( black_non_blocking_pawns.pawn_push_s ( pp ) | ( black_straight_blocking_pawns.pawn_push_s ( pp ) & black_straight_block_vectors ) ) & black_check_vectors_dep_check_count;

        /* Get general pawn attacks */
        const bitboard white_pawn_attacks = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne ) | bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw );
        const bitboard black_pawn_attacks = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se ) | bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw );

        /* Get pawn captures (legal pawn attacks).
         * Non-blocking pawns can attack without restriction.
         * Blocking pawns can attack only if they started and ended within a diagonal block vector.
         * If in check, ensure that the movements protected the king.
         */
        const bitboard white_pawn_captures_e = ( white_non_blocking_pawns.pawn_attack ( diagonal_compass::ne ) | ( white_diagonal_blocking_pawns.pawn_attack ( diagonal_compass::ne ) & white_diagonal_block_vectors ) ) & bb ( pcolor::black ) & white_legalize_attacks;
        const bitboard white_pawn_captures_w = ( white_non_blocking_pawns.pawn_attack ( diagonal_compass::nw ) | ( white_diagonal_blocking_pawns.pawn_attack ( diagonal_compass::nw ) & white_diagonal_block_vectors ) ) & bb ( pcolor::black ) & white_legalize_attacks;
        const bitboard black_pawn_captures_e = ( black_non_blocking_pawns.pawn_attack ( diagonal_compass::se ) | ( black_diagonal_blocking_pawns.pawn_attack ( diagonal_compass::se ) & black_diagonal_block_vectors ) ) & bb ( pcolor::white ) & black_legalize_attacks;
        const bitboard black_pawn_captures_w = ( black_non_blocking_pawns.pawn_attack ( diagonal_compass::sw ) | ( black_diagonal_blocking_pawns.pawn_attack ( diagonal_compass::sw ) & black_diagonal_block_vectors ) ) & bb ( pcolor::white ) & black_legalize_attacks;



        /* Union defence */
        white_partial_defence_union |= white_pawn_attacks;
        black_partial_defence_union |= black_pawn_attacks;

        /* Sum mobility */
        white_mobility += white_pawn_pushes.popcount () + white_pawn_captures_e.popcount () + white_pawn_captures_w.popcount ();
        black_mobility += black_pawn_pushes.popcount () + black_pawn_captures_e.popcount () + black_pawn_captures_w.popcount ();

        /* Incorporate the number of pawns into value */
        value += PAWN * ( bb ( pcolor::white, ptype::pawn ).popcount () - bb ( pcolor::black, ptype::pawn ).popcount () );

        /* Incorporate the number of cells generally attacked by pawns into value */
        value += PAWN_GENERAL_ATTACKS * ( white_pawn_attacks.popcount () - black_pawn_attacks.popcount () );

        /* Incorperate the number of pawns in the center to value */
        {
            const bitboard white_center_pawns = bb ( pcolor::white, ptype::pawn ) & white_center;
            const bitboard black_center_pawns = bb ( pcolor::black, ptype::pawn ) & black_center;
            value += CENTER_PAWNS * ( white_center_pawns.popcount () - black_center_pawns.popcount () );
        }

        /* Incororate the number of center cells generally attacked by pawns into value */
        {
            const bitboard white_center_defence = white_pawn_attacks & white_center;
            const bitboard black_center_defence = black_pawn_attacks & black_center;
            value += PAWN_CENTER_GENERAL_ATTACKS * ( white_center_defence.popcount () - black_center_defence.popcount () );
        }

        /* Incorporate isolated pawns, and those on semiopen files, into value */
        {
            const bitboard white_isolated_pawns = bb ( pcolor::white, ptype::pawn ) & ~( white_pawn_file_fill.shift ( compass::e ) | white_pawn_file_fill.shift ( compass::w ) );
            const bitboard black_isolated_pawns = bb ( pcolor::black, ptype::pawn ) & ~( black_pawn_file_fill.shift ( compass::e ) | black_pawn_file_fill.shift ( compass::w ) );
            value += ISOLATED_PAWNS * ( white_isolated_pawns.popcount () - black_isolated_pawns.popcount () );
            value += ISOLATED_PAWNS_ON_SEMIOPEN_FILES * ( ( white_isolated_pawns & white_semiopen_files ).popcount () - ( black_isolated_pawns & black_semiopen_files ).popcount () );
        }

        /* Incorporate the number of doubled pawns into value */
        {
            const bitboard white_doubled_pawns = bb ( pcolor::white, ptype::pawn ) & white_pawn_rear_span;
            const bitboard black_doubled_pawns = bb ( pcolor::black, ptype::pawn ) & black_pawn_rear_span;
            value += DOUBLED_PAWNS * ( white_doubled_pawns.popcount () - black_doubled_pawns.popcount () );
        }

        /* Incorporate the number of cells adjacent to enemy king, which are generally attacked by pawns, into value */
        {
            const bitboard white_pawn_defence_adj_op_king = white_pawn_attacks & black_king_span;
            const bitboard black_pawn_defence_adj_op_king = black_pawn_attacks & white_king_span;
            value += PAWN_GENERAL_ATTACKS_ADJ_OP_KING * ( white_pawn_defence_adj_op_king.popcount () - black_pawn_defence_adj_op_king.popcount () );
        }

        /* Incorporate phalanga into value */
        {
            const bitboard white_phalanga = bb ( pcolor::white, ptype::pawn ) & bb ( pcolor::white, ptype::pawn ).shift ( compass::e );
            const bitboard black_phalanga = bb ( pcolor::black, ptype::pawn ) & bb ( pcolor::black, ptype::pawn ).shift ( compass::e );
            value += PHALANGA * ( white_phalanga.popcount () - black_phalanga.popcount () );
        }

        /* Incorporate blocked passed pawns into value */
        {
            const bitboard white_blocked_passed_pawns = white_behind_passed_pawns.shift ( compass::n ).shift ( compass::n ) & ~bb ( pcolor::white, ptype::pawn ) & bb ();
            const bitboard black_blocked_passed_pawns = black_behind_passed_pawns.shift ( compass::s ).shift ( compass::s ) & ~bb ( pcolor::black, ptype::pawn ) & bb ();
            value += BLOCKED_PASSED_PAWNS * ( white_blocked_passed_pawns.popcount () - black_blocked_passed_pawns.popcount () );
        }
    }



    /* KING ATTACKS */

    /* Use the king position to look for possible attacks.
     * As well as using the partial defence union, check any left over king moves for check.
     */

    /* Scope the new bitboards */
    {
        /* Get all the king legal attacks (into empty space or an opposing piece).
         * The empty space must not be protected by the opponent, and the kings must not be left adjacent.
         * Note that some illegal attacks may be included here, if there are any opposing blocking sliding pieces.
         */
        bitboard white_king_attacks = white_king_span & ~black_king_span & ~bb ( pcolor::white ) & ~black_partial_defence_union;
        bitboard black_king_attacks = black_king_span & ~white_king_span & ~bb ( pcolor::black ) & ~white_partial_defence_union;

        /* Validate the remaining white king moves */
        if ( white_king_attacks.is_nonempty () ) 
        {
            /* Create a temporary bitboard */
            bitboard white_king_attacks_temp = white_king_attacks;

            /* Temporarily unset the king */
            const bitboard white_king = bb ( pcolor::white, ptype::king );
            get_bb ( pcolor::white )              &= ~white_king;
            get_bb ( pcolor::white, ptype::king ) &= ~white_king;

            /* Loop through the white king attacks to validate they don't lead to check */
            do {
                /* Get the position of the next king attack then use the position to determine if it is defended by the opponent */
                unsigned pos = white_king_attacks_temp.trailing_zeros_nocheck ();
                white_king_attacks.reset_if ( pos, is_protected ( pcolor::black, pos ) );
                white_king_attacks_temp.reset ( pos );
            } while ( white_king_attacks_temp );

            /* Reset the king */
            get_bb ( pcolor::white )              |= white_king;
            get_bb ( pcolor::white, ptype::king ) |= white_king;
        }

        /* Validate the remaining black king moves */
        if ( black_king_attacks.is_nonempty () ) 
        {
            /* Create a temporary bitboard */
            bitboard black_king_attacks_temp = black_king_attacks;

            /* Temporarily unset the king */
            const bitboard black_king = bb ( pcolor::black, ptype::king );
            get_bb ( pcolor::black )              &= ~black_king;
            get_bb ( pcolor::black, ptype::king ) &= ~black_king;

            /* Loop through the white king attacks to validate they don't lead to check */
            do {
                /* Get the position of the next king attack then use the position to determine if it is defended by the opponent */
                unsigned pos = black_king_attacks_temp.trailing_zeros_nocheck ();
                black_king_attacks.reset_if ( pos, is_protected ( pcolor::white, pos ) );
                black_king_attacks_temp.reset ( pos );
            } while ( black_king_attacks_temp );

            /* Reset the king */
            get_bb ( pcolor::black )              |= black_king;
            get_bb ( pcolor::black, ptype::king ) |= black_king;
        }

        /* Calculate king queen fill. Flood fill using pp and queen attack lookup as a propagator. */
        const bitboard white_king_queen_fill = bb ( pcolor::white, ptype::king ).straight_flood_fill ( bitboard::straight_attack_lookup ( white_king_pos ) & pp )
                                             | bb ( pcolor::white, ptype::king ).diagonal_flood_fill ( bitboard::diagonal_attack_lookup ( white_king_pos ) & pp );
        const bitboard black_king_queen_fill = bb ( pcolor::black, ptype::king ).straight_flood_fill ( bitboard::straight_attack_lookup ( black_king_pos ) & pp )
                                             | bb ( pcolor::black, ptype::king ).diagonal_flood_fill ( bitboard::diagonal_attack_lookup ( black_king_pos ) & pp );



        /* Sum mobility */
        white_mobility += white_king_attacks.popcount ();
        black_mobility += black_king_attacks.popcount ();

        /* Incorporate the king queen mobility into value.
         * It does not matter that we are using the fills instead of spans, since the fact both the fills include their kings will cancel out.
         */
        value += KING_QUEEN_MOBILITY * ( white_king_queen_fill.popcount () - black_king_queen_fill.popcount () );
    }

    /* CHECKMATE */

    /* Return maximum if one color has no mobility and the other does.
     * On a stalemate, currently continue normally.
     */
    if ( ( white_mobility == 0 ) & ( black_mobility != 0 ) ) return ( pc == pcolor::white ? -CHECKMATE :  CHECKMATE );
    if ( ( black_mobility == 0 ) & ( white_mobility != 0 ) ) return ( pc == pcolor::white ?  CHECKMATE : -CHECKMATE );



    /* FINISH ADDING TO VALUE */

    /* Mobility */
    value += MOBILITY * ( white_mobility - black_mobility );

    /* Attacks by restrictives on center */
    value += CENTER_LEGAL_ATTACKS_BY_RESTRICTIVES * center_legal_attacks_by_restrictives_diff;

    /* Captures on straight pieces by diagonal pieces and knights */
    value += DIAGONAL_OR_KNIGHT_CAPTURE_ON_STRAIGHT_PIECES * diagonal_or_knight_captures_on_straight_diff;

    /* Knight and queen exist */
    {
        const bool white_knight_and_queen_exist = bb ( pcolor::white, ptype::knight ).is_nonempty () & bb ( pcolor::white, ptype::queen ).is_nonempty ();
        const bool black_knight_and_queen_exist = bb ( pcolor::black, ptype::knight ).is_nonempty () & bb ( pcolor::black, ptype::queen ).is_nonempty ();
        value += KNIGHT_AND_QUEEN_EXIST * ( white_knight_and_queen_exist - black_knight_and_queen_exist );
    }

    /* Castling */
    value += CASTLE_MADE * ( castle_made ( pcolor::white ) - castle_made ( pcolor::black ) );
    value += CASTLE_LOST * ( castle_lost ( pcolor::white ) - castle_lost ( pcolor::black ) );



    /* Return the value */
    return ( pc == pcolor::white ? value : -value );
}



/* ALPHA-BETA SEARCH */

/** @name  alpha_beta_search
 * 
 * @brief  Set up and apply the alpha beta search
 * @param  pc: The color who's move it is next
 * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
 * @param  end_point: The time point at which the search should be ended, never by default.
 * @return An array of moves and their values
 */
chess::chessboard::move_list_t chess::chessboard::alpha_beta_search ( const pcolor pc, const unsigned depth, const std::chrono::steady_clock::time_point end_point )
{
    /* Allocate new memory if it does not already exist. */
    if ( !ab_working ) ab_working = new ab_working_t;

    /* Allocate the memory for moves and killer moves */
    ab_working->moves.resize ( 32 );
    ab_working->killer_moves.resize ( 32 );

    /* Reserve memory for root moves */
    ab_working->root_moves.reserve ( 128 );

    /* Call the internal method */
    alpha_beta_search_internal ( pc, depth, end_point );

    /* Extract the move list */
    move_list_t moves = std::move ( ab_working->root_moves );

    /* Shrink to fit */
    moves.shrink_to_fit ();

    /* Order the moves */
    std::sort ( moves.begin (), moves.end (), [] ( const auto& lhs, const auto& rhs ) { return lhs.second > rhs.second; } );
    
    /* Delete the memory */
    delete ab_working; ab_working = nullptr;

    /* Return the moves */
    return moves;
}

/** @name  alpha_beta_iterative_deepening
 * 
 * @brief  Apply an alpha beta search over a range of depths
 * @param  pc: The color who's move it is next
 * @param  min_depth: The lower bound of the depths to try
 * @param  max_depth: The upper bound of the depths to try
 * @param  end_point: The time point at which the search should be ended
 * @param  threads: The number of threads to run simultaneously, 0 by default
 * @return An array of moves and their values
 */
chess::chessboard::move_list_t chess::chessboard::alpha_beta_iterative_deepening ( const pcolor pc, const unsigned min_depth, const unsigned max_depth, const std::chrono::steady_clock::time_point end_point, unsigned threads )
{
    /* If threads == 0, increase it to 1 */
    if ( threads == 0 ) threads = 1;

    /* Create an array of chessboards and futures for each depth */
    std::vector<chessboard> cbs { max_depth - min_depth + 1, * this };
    std::vector<std::future<move_list_t>> fts { max_depth - min_depth + 1 };

    /* Set of the first set of futures */
    for ( unsigned i = 0; i < threads && min_depth + i <= max_depth; ++i ) 
        fts.at ( i ) = std::async ( std::launch::async, &chess::chessboard::alpha_beta_search, &cbs.at ( i ), pc, min_depth + i, end_point );

    /* The running moves list */
    move_list_t deepest_moves;

    /* Wait for each future in turn */
    for ( unsigned i = 0; min_depth + i <= max_depth; ++i )
    {
        /* If the future is invalid, break */
        if ( !fts.at ( i ).valid () ) break;

        /* Get the next future */
        move_list_t new_moves = fts.at ( i ).get ();

        /* Only accept the new moves if not passed the end point */
        if ( std::chrono::steady_clock::now () < end_point ) 
        {
            /* Set the deepest moves */
            deepest_moves = std::move ( new_moves );

            /* Possibly start the next thread */
            if ( min_depth + i + threads <= max_depth ) 
                fts.at ( i + threads ) = std::async ( std::launch::async, &chess::chessboard::alpha_beta_search, &cbs.at ( i + threads ), pc, min_depth + i + threads, end_point );
        }
    }
    
    /* Return the moves from the deepest complete search */
    return deepest_moves;
}

/** @name  alpha_beta_search_internal
 * 
 * @brief  Apply an alpha-beta search to a given depth.
 *         Note that although is non-const, a call to this function which does not throw will leave the object unmodified.
 * @param  pc: The color who's move it is next
 * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
 * @param  end_point: The time point at which the search should be ended, never by default.
 * @param  fd_depth: The forwards depth, defaults to 0 and should always be 0.
 * @param  alpha: The maximum value pc has discovered, defaults to -10000.
 * @param  beta:  The minimum value not pc has discovered, defaults to 10000.
 * @param  has_null: Whether a null move has previously been applied, defaults to false.
 * @param  quiesce: Whether quiescence has started, defaults to false.
 * @return int
 */
int chess::chessboard::alpha_beta_search_internal ( const pcolor pc, unsigned depth, std::chrono::steady_clock::time_point end_point, unsigned fd_depth, int alpha, int beta, const bool has_null, bool quiesce )
{
    /* CONSTANTS */

    /* Set the range within states will be searched for in the transposition table */
    constexpr unsigned ttable_min_search_fd_depth = 3, ttable_max_search_fd_depth = 6;

    /* Set the range within new states will be stored in the transposition table */
    constexpr unsigned ttable_min_store_fd_depth = 1, ttable_max_store_fd_depth = 4;

    /* The change in depth for a null move */
    constexpr unsigned null_move_change_depth = 3;

    /* The maximum search depth for a null move */
    constexpr unsigned null_move_max_search_depth = 7;

    /* The number of pieces required for a null move */
    constexpr unsigned null_move_min_pieces = 7;

    /* The minimum depth at which an endpoint cutoff is noticed */
    constexpr unsigned end_point_cutoff_min_depth = 5;

    /* Create a constexpr list of sliding ptypes */
    constexpr ptype sliding_pts [] = { ptype::bishop, ptype::rook, ptype::queen };

    /* Create a constexpr list of what order pieces should be examined for different moves */
    constexpr ptype capture_order [] = { ptype::pawn, ptype::knight, ptype::bishop, ptype::rook, ptype::queen, ptype::king };
    constexpr ptype move_order []    = { ptype::knight, ptype::queen, ptype::rook, ptype::bishop, ptype::pawn, ptype::king };

    /* The top and bottom ranks of the board (to detect pawns being queened) */
    constexpr bitboard top_and_bottom_ranks { bitboard::masks::rank_1 | bitboard::masks::rank_8 };



    /* SETUP */

    /* Store the current best value.
     * Losing sooner is worse, which the minus depth accounts for
     */
    int best_value = -5000 - depth;

    /* Get the other player */
    const pcolor npc = other_color ( pc );

    /* Get the check info of pc */
    const check_info_t check_info = get_check_info ( pc );
    const bitboard check_vectors = check_info.check_vectors, block_vectors = check_info.block_vectors;

    /* Throw if the opposing king is in check */
    //if ( is_in_check ( npc ) ) throw std::runtime_error { "Opposing color is in check in alpha_beta_search_internal ()." };

    /* Get king position */
    const unsigned king_pos = bb ( pc, ptype::king ).trailing_zeros_nocheck ();

    /* Get the straight and diagonal block vectors */
    const bitboard straight_block_vectors = block_vectors & bitboard::straight_attack_lookup ( king_pos );
    const bitboard diagonal_block_vectors = block_vectors & bitboard::diagonal_attack_lookup ( king_pos );

    /* See evaluate () # Setup for more info */
    const bitboard check_vectors_dep_check_count = check_vectors.all_if ( !check_vectors ).only_if ( ( check_vectors & bb ( npc ) ).popcount () < 2 );

    /* Get the primary and secondary propagator sets */
    const bitboard pp = ~bb (), sp = ~bb ( pc );

    /* Get the opposing concentation. True for north of board, false for south. */
    const bool opposing_conc = ( bb ( npc ) & bitboard { 0xffffffff00000000 } ).popcount () >= ( bb ( npc ) & bitboard { 0x00000000ffffffff } ).popcount ();

    /* Whether should add to transposition table */
    const bool ttable_store = !has_null && fd_depth <= ttable_max_store_fd_depth && fd_depth >= ttable_min_store_fd_depth;

    /* Whether should try a null move.
     * Must not have used one before.
     * Must not be in check.
     * Reducing depth by null_move_change_depth must leave a depth of at least 2, and at most null_move_max_search_depth.
     * There must be at least null_move_min_pieces pieces left for this color.
     * There must be pieces for this color other than the king and pawns.
     */
    const bool try_null_move = !has_null && !check_vectors && depth >= 2 + null_move_change_depth && depth <= null_move_max_search_depth + null_move_change_depth && bb ( pc ).popcount () >= null_move_min_pieces && ( bb ( pc, ptype::queen ) | bb ( pc, ptype::rook ) | bb ( pc, ptype::bishop ) | bb ( pc, ptype::knight ) );

    /* If fd_depth is greater than the size of the vectors in ab_working, increase their allocated memory */
    if ( fd_depth >= ab_working->moves.size () ) { ab_working->moves.resize ( ab_working->moves.size () * 2 ); ab_working->killer_moves.resize ( ab_working->killer_moves.size () * 2 ); }

    /* Clear the move sets */
    for ( auto& moves : ab_working->moves.at ( fd_depth ) ) moves.clear ();

    /* Enums to store killer move states */
    enum class killer_move_state_t { unfound, possible, failed };
    std::pair<killer_move_state_t, killer_move_state_t> killer_move_states { killer_move_state_t::unfound, killer_move_state_t::unfound };

    /* Get the current killer moves */
    auto killer_moves = ab_working->killer_moves.at ( fd_depth );



    /* CHECK FOR LEAF */

    /* If at depth zero, start or continue with quiescence */
    int static_eval = 0;
    if ( depth == 0 )
    {
        /* If in check increase the depth by 1 */
        if ( check_vectors ) depth++;

        /* Set flag */
        quiesce = true;

        /* Get static evaluation */
        static_eval = evaluate ( pc );

        /* Return immediately if static evaluation is greater than beta, return the evaluation */
        if ( static_eval >= beta ) return static_eval;

        /* Tune alpha to the static evaluation */
        alpha = std::max ( alpha, static_eval );
    }


    
    /* TRY A LOOKUP */

    /* Only try if in the specified depth range */
    if ( !has_null && fd_depth <= ttable_max_search_fd_depth && fd_depth >= ttable_min_search_fd_depth )
    {
        /* Try to find the state */
        auto search_it = ab_working->ttable.find ( ab_state_t { * this, pc } );

        /* If is found, return the value stored */
        if ( search_it != ab_working->ttable.end () ) return search_it->second;
    }



    /* TRY NULL MOVE */

    /* If null move is possible, try it */
    if ( try_null_move )
    {
        /* Apply the null move */
        int score = -alpha_beta_search_internal ( npc, depth - null_move_change_depth, end_point, fd_depth + 2, -beta, -beta + 1, true, quiesce );

        /* If proved successful, return */
        if ( score >= beta ) return score;
    }



    /* MOVE LOOP FUNCTOR */

    /** @name  apply_moves
     * 
     * @brief  loops through moves and recursively calls alpha_beta_search_internal on each of them
     * @param  pt: The type of the piece currently in focus
     * @param  pos_bb: A singleton bitboard for the piece currently in focus
     * @param  moves: The possible moves for that piece
     * @return boolean, true for an alpha-beta cutoff, false otherwise
     */
    auto apply_moves = [ & ] ( ptype pt, bitboard pos_bb, bitboard moves ) -> bool
    {
        /* Unset pos in the board state */
        get_bb ( pc )     &= ~pos_bb;
        get_bb ( pc, pt ) &= ~pos_bb;



        /* MOVE LOOP */

        /* Iterate through the individual moves */
        while ( moves )
        {
            /* Get the position and singleton bitboard of the next move.
             * Choose motion towards the opposing color.
             */
            const unsigned move_pos = ( opposing_conc ? 63 - moves.leading_zeros_nocheck () : moves.trailing_zeros_nocheck () );
            const bitboard move_pos_bb = singleton_bitboard ( move_pos );

            /* Unset this bit in the set of moves */
            moves &= ~move_pos_bb;

            /* Set the new bits for the piece move */
            get_bb ( pc )     |= move_pos_bb;
            get_bb ( pc, pt ) |= move_pos_bb; 

            /* Get any enemy piece captured by the move */
            const ptype capture_pt = find_type ( npc, move_pos_bb );

            /* If there is a capture, unset those bits */
            if ( capture_pt != ptype::no_piece )
            {
                get_bb ( npc )             &= ~move_pos_bb;
                get_bb ( npc, capture_pt ) &= ~move_pos_bb;
            }

            /* Queen pawns */
            const bitboard queened_pawns = bb ( pc, ptype::pawn ) & top_and_bottom_ranks;
            if ( queened_pawns )
            {
                bb ( pc, ptype::queen ) |=  queened_pawns;
                bb ( pc, ptype::pawn  ) &= ~queened_pawns;
            }

            /* Recursively call to get the value for this move.
            * Switch around and negate alpha and beta, since it is the other player's turn.
            * Since the next move is done by the other player, negate the value.
            */
            int new_value = -alpha_beta_search_internal ( npc, ( depth != 0 ? depth - 1 : 0 ), end_point, fd_depth + 1, -beta, -alpha, has_null, quiesce );

            /* Unset the bits changed for the piece move */
            get_bb ( pc )     &= ~move_pos_bb;
            get_bb ( pc, pt ) &= ~move_pos_bb; 

            /* If there was a capture, reset those bits */
            if ( capture_pt != ptype::no_piece )
            {
                get_bb ( npc )             |= move_pos_bb;
                get_bb ( npc, capture_pt ) |= move_pos_bb;
            }

            /* Unqueen queened pawns */
            if ( queened_pawns )
            {
                bb ( pc, ptype::queen ) &= ~queened_pawns;
                bb ( pc, ptype::pawn  ) |=  queened_pawns;
            }

            /* If past the end point, return */
            if ( depth >= end_point_cutoff_min_depth && std::chrono::steady_clock::now () > end_point ) 
            {
                /* Reset pos in board state */
                get_bb ( pc )     |= pos_bb;
                get_bb ( pc, pt ) |= pos_bb;

                /* Return true to cause cutoff */
                return true;
            }

            /* If at the parent node, add to the root moves */
            if ( fd_depth == 0 ) ab_working->root_moves.push_back ( std::make_pair ( move_t { pc, pt, pos_bb, move_pos_bb }, new_value ) );

            /* If the new value is greater than the best value, then reassign the best value.
            * Further check if the new best value is greater than alpha, if so reassign alpha.
            * If alpha is now greater than beta, return true due to an alpha beta cutoff.
            */
            best_value = std::max ( best_value, new_value );
            alpha = std::max ( alpha, best_value );
            if ( alpha >= beta )
            {
                /* Reset pos in board state */
                get_bb ( pc )     |= pos_bb;
                get_bb ( pc, pt ) |= pos_bb;

                /* Create the move */
                const move_t move { pc, pt, pos_bb, move_pos_bb };

                /* If the most recent killer move is not correct, swap the pair */
                if ( killer_moves.first != move )
                {
                    /* Swap */
                    std::swap ( killer_moves.first, killer_moves.second );

                    /* If the most recent killer move is still not correct, replace the most recent killer move */
                    if ( killer_moves.first != move ) killer_moves.first = move;

                    /* Copy over the new killer moves */
                    ab_working->killer_moves.at ( fd_depth ) = killer_moves; 
                }

                /* If is flagged to do so, add to the transposition table */
                if ( ttable_store ) ab_working->ttable.insert ( std::make_pair ( ab_state_t { * this, pc }, best_value ) );

                /* Return */
                return true;
            }
        }



        /* Reset pos in board state */
        get_bb ( pc )     |= pos_bb;
        get_bb ( pc, pt ) |= pos_bb;

        /* Return false */
        return false;
    };



    /* KILLER MOVE FUNCTOR */

    /** @name  try_killer_move
     * 
     * @brief  Check whether a killer move is possible and apply it if so
     * @param  pt: The type of the piece currently in focus
     * @param  pos_bb: A singleton bitboard for the piece currently in focus
     * @param  moves: The moves for that piece
     * @return boolean, true for an alpha-beta cutoff, false otherwise
     */
    auto try_killer_move = [ & ] ( ptype pt, bitboard pos_bb, bitboard& moves ) -> bool
    {
        /* If the first killer move is unfound, try to find it now */
        if ( killer_move_states.first == killer_move_state_t::unfound ) if ( pt == killer_moves.first.pt && pos_bb == killer_moves.first.from_bb )
        {
            /* The piece was found, and if it is possible try it immediately */
            if ( moves & killer_moves.first.to_bb ) if ( apply_moves ( pt, pos_bb, killer_moves.first.to_bb ) ) return true;

            /* Unset that move */
            moves &= ~killer_moves.first.to_bb;

            /* Set that the first killer move failed */
            killer_move_states.first = killer_move_state_t::failed;
        }

        /* If the second killer move is unfound, try to find it now */
        if ( killer_move_states.second == killer_move_state_t::unfound ) if ( pt == killer_moves.second.pt && pos_bb == killer_moves.second.from_bb )
        {
            /* The piece was found, so test if it is possible */
            if ( moves & killer_moves.second.to_bb ) killer_move_states.second == killer_move_state_t::possible; else killer_move_states.second == killer_move_state_t::failed;
        }

        /* If the first killer move has failed and the second has been marked possible, try it now */
        if ( killer_move_states.first == killer_move_state_t::failed && killer_move_states.second ==  killer_move_state_t::possible )
        {
            /* Try the move */
            if ( apply_moves ( pt, pos_bb, killer_moves.second.to_bb ) ) return true;

            /* Unset that move */
            moves &= ~killer_moves.second.to_bb;

            /* Set to failed */
            killer_move_states.second = killer_move_state_t::failed;
        }

        /* Return false */
        return false;
    };





    /* PAWNS */
    {
        /* Get the pawns */
        bitboard pawns = bb ( pc, ptype::pawn ); 



        /* PIECE LOOP */

        /* Iterate through the pawns */
        while ( pawns )
        {
            /* Get the position and singleton bitboard of the next pawn.
             * Favour the further away pawns to encourage them to move towards the other color.
             */
            const unsigned pos = ( opposing_conc ? pawns.trailing_zeros_nocheck () : 63 - pawns.leading_zeros_nocheck () );
            const bitboard pos_bb = singleton_bitboard ( pos );

            /* Unset this bit in the set of pawns */
            pawns &= ~pos_bb;

            /* Prepare to union pawn moves */
            bitboard moves;

            /* PAWN ATTACKS */

            /* If is on a straight block vector, cannot be a valid pawn attack */
            if ( straight_block_vectors.is_disjoint ( pos_bb ) )
            {
                /* Get the attacks, and ensure that they protected the king */
                bitboard attacks = ( pc == pcolor::white ? pos_bb.pawn_any_attack_n ( bb ( npc ) ) : pos_bb.pawn_any_attack_s ( bb ( npc ) ) ) & check_vectors_dep_check_count;

                /* If is on a diagonal block vector, ensure the captures stayed on the block vector */
                if ( pos_bb & diagonal_block_vectors ) attacks &= diagonal_block_vectors;

                /* Union moves */
                moves |= attacks;                
            }

            /* PAWN PUSHES */

            /* If is on a straight block vector, cannot be a valid pawn push */
            if ( diagonal_block_vectors.is_disjoint ( pos_bb ) )
            {
                /* Get the pushes, and ensure that they protected the king */
                bitboard pushes = ( pc == pcolor::white ? pos_bb.pawn_push_n ( pp ) : pos_bb.pawn_push_s ( pp ) ) & check_vectors_dep_check_count;

                /* If is on a straight block vector, ensure the pushes stayed on the block vector */
                if ( pos_bb & straight_block_vectors ) pushes &= straight_block_vectors;

                /* Union moves */
                moves |= pushes;
            }

            /* Try the killer moves */
            if ( try_killer_move ( ptype::pawn, pos_bb, moves ) ) return best_value;

            /* Store pawn moves */
            ab_working->moves.at ( fd_depth ).at ( static_cast<int> ( ptype::pawn ) ).push_back ( std::make_pair ( pos_bb, moves ) );
        }
    }





    /* KNIGHTS */
    {
        /* Get the knights */
        bitboard knights = bb ( pc, ptype::knight );

        /* PIECE LOOP */

        /* Iterate through the knights */
        while ( knights )
        {
            /* Get the position and singleton bitboard of the next knight.
             * Favour the further away knights to encourage them to move towards the other color.
             */
            const unsigned pos = ( opposing_conc ? knights.trailing_zeros_nocheck () : 63 - knights.leading_zeros_nocheck () );
            const bitboard pos_bb = singleton_bitboard ( pos );

            /* Unset this bit in the set of knights */
            knights &= ~pos_bb;

            /* If the knight is blocking, continue */
            if ( pos_bb & block_vectors ) continue;

            /* Get the attacks, ensuring they protected the king */
            bitboard attacks = bitboard::knight_attack_lookup ( pos ) & sp & check_vectors_dep_check_count;

            /* Try the killer moves */
            if ( try_killer_move ( ptype::knight, pos_bb, attacks ) ) return best_value;

            /* Store knight attacks */
            ab_working->moves.at ( fd_depth ).at ( static_cast<int> ( ptype::knight ) ).push_back ( std::make_pair ( pos_bb, attacks ) );
        }
    }





    /* SLIDING PIECES */

    /* TYPE LOOP */

    /* Iterate through the different types of sliding piece */
    for ( unsigned i = 0; i < 3; ++i )
    {
        /* Get the ptype */
        const ptype pt = sliding_pts [ i ];

        /* Get a temporary version of the bitboard for this type */
        bitboard pt_bb = bb ( pc, pt );



        /* PIECE LOOP */

        /* Loop through each piece while there are any left */
        while ( pt_bb )
        {
            /* Get the position and singleton bitboard of the next piece */
            const unsigned pos = pt_bb.trailing_zeros_nocheck ();
            const bitboard pos_bb = singleton_bitboard ( pos );

            /* Unset this bit in the set of pieces of this type */
            pt_bb &= ~pos_bb;

            /* Get the straight and diagonal attack lookups */
            bitboard straight_attack_lookup = bitboard::straight_attack_lookup ( pos );
            bitboard diagonal_attack_lookup = bitboard::diagonal_attack_lookup ( pos );

            /* If the piece is blocking, then it is possible the piece may be immobile.
             * First check if it is on a straight block vector.
            */
            if ( straight_block_vectors & pos_bb ) [[ unlikely ]]
            {
                /* If in check, or is a bishop, continue */
                if ( check_vectors.is_empty () | ( pt == ptype::bishop ) ) continue;

                /* Now we know the blocking piece can move, restrict the propagators accordingly */
                straight_attack_lookup &= straight_block_vectors;
                diagonal_attack_lookup.empty ();
            } else

            /* Else check if is on a diagonal block vector */
            if ( diagonal_block_vectors & pos_bb ) [[ unlikely ]]
            {
                /* If in check, or is a rook, continue */
                if ( check_vectors.is_empty () | ( pt == ptype::rook ) ) continue;

                /* Now we know the blocking piece can move, restrict the propagators accordingly */
                diagonal_attack_lookup &= diagonal_block_vectors;
                straight_attack_lookup.empty ();
            }

            /* Set attacks initially to empty */
            bitboard attacks;

            /* If is not a bishop, apply a straight flood span.
             * If is not a rook, apply a diagonal flood span.
             */
            if ( pt != ptype::bishop ) attacks |= pos_bb.straight_flood_span ( pp & straight_attack_lookup, sp & straight_attack_lookup );
            if ( pt != ptype::rook   ) attacks |= pos_bb.diagonal_flood_span ( pp & diagonal_attack_lookup, sp & diagonal_attack_lookup );

            /* Ensure that attacks protected the king */
            attacks &= check_vectors_dep_check_count;

            /* Try the killer moves */
            if ( try_killer_move ( pt, pos_bb, attacks ) ) return best_value;

            /* Store sliding piece attacks */
            ab_working->moves.at ( fd_depth ).at ( static_cast<int> ( pt ) ).push_back ( std::make_pair ( pos_bb, attacks ) );
        }
    }





    /* KING */
    {
        /* Get the king */
        const bitboard king = bb ( pc, ptype::king );

        /* Lookup the king attacks */
        bitboard attacks = bitboard::king_attack_lookup ( king_pos ) & sp;

        /* Unset the king */
        get_bb ( pc ) &= ~king;
        get_bb ( pc, ptype::king ).empty ();

        /* Iterate through the attacks and make sure that they are not protected */
        bitboard attacks_temp = attacks;
        while ( attacks_temp )
        {
            /* Get the next test position */
            const unsigned test_pos = attacks_temp.trailing_zeros_nocheck ();
            attacks_temp.reset ( test_pos );

            /* If is protected, reset in attacks */
            attacks.reset_if ( test_pos, is_protected ( npc, test_pos ) );
        }

        /* Reset the king */
        get_bb ( pc )              |= king;
        get_bb ( pc, ptype::king ) |= king;

        /* Try the killer moves */
        if ( try_killer_move ( ptype::king, king, attacks ) ) return best_value;

        /* Store attacks */
        ab_working->moves.at ( fd_depth ).at ( static_cast<int> ( ptype::king ) ).push_back ( std::make_pair ( king, attacks ) );
    }





    /* SEARCH THROUGH MOVES */

    /* Look for pawn moves that queen that pawn */
    for ( auto& moves : ab_working->moves.at ( fd_depth ).at ( static_cast<int> ( ptype::pawn ) ) )
    {
        /* Apply the move, then remove those bits */
        if ( apply_moves ( ptype::pawn, moves.first, moves.second & top_and_bottom_ranks ) ) return best_value; 
        moves.second &= ~top_and_bottom_ranks;
    }

    /* Look for captures on non-pawns. Less valuable pieces capturing is more likely to be profitable. */
    for ( unsigned i = 0; i < 6; ++i ) for ( auto& moves : ab_working->moves.at ( fd_depth ).at  ( static_cast<int> ( capture_order [ i ] ) ) )
        if ( apply_moves ( capture_order [ i ], moves.first, moves.second & bb ( npc ) & ~bb ( npc, ptype::pawn ) ) ) return best_value;
            
    /* Look for captures on pawns. Less valuable pieces capturing is more likely to be profitable. */
    for ( unsigned i = 0; i < 6; ++i ) for ( auto& moves : ab_working->moves.at ( fd_depth ).at  ( static_cast<int> ( capture_order [ i ] ) ) )
        if ( apply_moves ( capture_order [ i ], moves.first, moves.second & bb ( npc, ptype::pawn ) ) ) return best_value;

    /* If at depth 0, don't consider non-captures */
    if ( depth != 0 )

    /* Look for other moves. Knights are more likely to have a big effect, followed by queens, rooks then bishops. */
    for ( unsigned i = 0; i < 6; ++i ) for ( auto& moves : ab_working->moves.at ( fd_depth ).at ( static_cast<int> ( move_order [ i ] ) ) )
        if ( apply_moves ( move_order [ i ], moves.first, moves.second & ~bb ( npc ) ) ) return best_value;





    /* FINALLY */

    /* If is flagged to do so, add to the transposition table */
    if ( ttable_store ) ab_working->ttable.insert ( std::make_pair ( ab_state_t { * this, pc }, best_value ) );

    /* Return the best value */
    return best_value;
}