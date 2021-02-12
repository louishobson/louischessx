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
        const bitboard mask { 1ull << ( i ^ 56 ) };
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
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( straight_dir ), king_pos ) & op_straight )
        {
            /* Span the king in the current direction */
            const bitboard king_span = king.rook_attack ( straight_dir, pp, sp );

            /* Get the blocking pieces */
            const bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.check_vectors |= king_span.only_if ( blocking.is_empty () );
            check_info.block_vectors |= king_span.only_if ( blocking.is_singleton () );
        }

        /* Increment compass */
        straight_dir = compass_next ( straight_dir );
    }

    /* Iterate through the diagonal compass to see if those sliding pieces could be attacking */
    if ( bitboard::diagonal_attack_lookup ( king_pos ) & op_diagonal )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( diagonal_dir ), king_pos ) & op_diagonal ) 
        {
            /* Span the king in the current direction */
            const bitboard king_span = king.bishop_attack ( diagonal_dir, pp, sp );

            /* Get the blocking pieces */
            const bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.check_vectors |= king_span.only_if ( blocking.is_empty () );
            check_info.block_vectors |= king_span.only_if ( blocking.is_singleton () );
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
    const bitboard pos_bb ( 1ull << pos );

    /* Get the positions of the friendly straight and diagonal pieces */
    const bitboard fr_straight = bb ( pc, ptype::queen ) | bb ( pc, ptype::rook );
    const bitboard fr_diagonal = bb ( pc, ptype::queen ) | bb ( pc, ptype::bishop );

    /* Get the primary propagator.
     * Primary propagator of not non-occupied will mean the span will stop at the first piece.
     * The secondary propagator of friendly pieces means the span will include a friendly piece if found.
     */
    const bitboard pp = ~bb ();
    const bitboard sp = bb ( pc );



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
    if ( bitboard::straight_attack_lookup ( pos ) & fr_straight )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( straight_dir ), pos ) & fr_straight )
            if ( pos_bb.rook_attack ( straight_dir, pp, sp ) & fr_straight ) return true;      

        /* Increment compass */
        straight_dir = compass_next ( straight_dir );
    }

    /* Iterate through the diagonal compass to see if those sliding pieces could be defending */
    if ( bitboard::diagonal_attack_lookup ( pos ) & fr_diagonal )
    #pragma GCC unroll 4
    for ( unsigned i = 0; i < 4; ++i )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected  */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( diagonal_dir ), pos ) & fr_diagonal )
            if ( pos_bb.bishop_attack ( diagonal_dir, pp, sp ) & fr_diagonal ) return true;

        /* Increment compass */
        diagonal_dir = compass_next ( diagonal_dir );
    }



    /* Return false */
    return false;    
}



/** @name  evaluate
 * 
 * @brief  Symmetrically evaluate the board state
 * @param  pc: The color who's move it is next.
 *         This must be the piece who's move it is next, in order to detect moves that are in check.
 * @return Integer value
 */
int chess::chessboard::evaluate ( pcolor pc ) const
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
     * Mobility is the number of distinct strictly legal moves. If a mobility is zero, that means that color is in checkmate.W
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
    constexpr int CHECKMATE { std::numeric_limits<int16_t>::max () }; // If true


    
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
    if ( ( pc == pcolor::white ? black_check_count : white_check_count ) != 0 ) [[ unlikely ]] throw std::runtime_error { "Opposing color in check in evaluate ()." };
    
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

    /* Get the restrictives */
    const bitboard white_restrictives = bb ( pcolor::white, ptype::bishop ) | bb ( pcolor::white, ptype::rook ) | bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::knight );
    const bitboard black_restrictives = bb ( pcolor::black, ptype::bishop ) | bb ( pcolor::black, ptype::rook ) | bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::knight );
    const bitboard restrictives = white_restrictives | black_restrictives;

    /* Set the primary propagator such that all empty cells are set */
    const bitboard pp = ~( bb ( pcolor::white ) | bb ( pcolor::black ) );

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
     * If there are blocking sliding pieces, the opponent's king's attacks must be individually inspected to test if they are legal.
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

    /* Whether each color has any blocking sliding pieces.
     * See partial defence unions above as to why these booleans are important.
     */
    bool white_has_blocking_sliding_pieces = false, black_has_blocking_sliding_pieces = false;



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

            /* Remove attacks on friendly pieces and ensure movements protected the king to get the legal attacks */
            white_straight_attacks &= ~bb ( pcolor::white ) & white_check_vectors_dep_check_count; 
            white_diagonal_attacks &= ~bb ( pcolor::white ) & white_check_vectors_dep_check_count;
            black_straight_attacks &= ~bb ( pcolor::black ) & black_check_vectors_dep_check_count;
            black_diagonal_attacks &= ~bb ( pcolor::black ) & black_check_vectors_dep_check_count;

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
            center_legal_attacks_by_restrictives_diff -= ( black_straight_attacks & black_center ).popcount () - ( black_diagonal_attacks & black_center ).popcount ();

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
     */

    /* White blocking pieces */
    if ( white_check_count == 0 )
    {
        /* Get the blocking pieces which can move */
        bitboard blocking_attacks =
            ( ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook   ) ) & white_straight_block_vectors ) |
            ( ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::bishop ) ) & white_diagonal_block_vectors );

        /* Only continue if there were appropriate sliding pieces */
        if ( blocking_attacks.is_nonempty () ) 
        {
            /* Set boolean flag */
            white_has_blocking_sliding_pieces = true;

            /* Flood fill the blocking pieces along the block vectors, which will give legal attacks */
            blocking_attacks = blocking_attacks.flood_fill ( white_block_vectors );

            /* Sum rook attacks on open and semiopen files */
            straight_legal_attacks_open_diff += ( blocking_attacks & white_straight_block_vectors & open_files ).popcount ();
            straight_legal_attacks_semiopen_diff += ( blocking_attacks & white_straight_block_vectors & white_semiopen_files ).popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff += ( blocking_attacks & white_center ).popcount ();

            /* Union restrictives attacked by diagonal pieces */
            restrictives_legally_attacked_by_white_diagonal_pieces |= restrictives & blocking_attacks;

            /* Get the number of diagonal restricted captures */
            diagonal_restricted_captures_diff += ( blocking_attacks & white_diagonal_block_vectors & black_restrictives ).popcount ();

            /* Sum the legal captures on enemy straight pieces by diagonal pieces */
            diagonal_or_knight_captures_on_straight_diff += ( blocking_attacks & white_diagonal_block_vectors & ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook ) ) ).popcount ();

            /* Union defence (at this point the defence union becomes partial) */
            white_partial_defence_union |= blocking_attacks | bb ( pcolor::white, ptype::king );

            /* Sum mobility */
            white_mobility += blocking_attacks.popcount ();
        }
    }

    /* Black blocking pieces */
    if ( black_check_count == 0 )
    {
        /* Get the blocking pieces which can move */
        bitboard blocking_attacks =
            ( ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook   ) ) & black_straight_block_vectors ) |
            ( ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::bishop ) ) & black_diagonal_block_vectors );

        /* Only continue if there were appropriate sliding pieces */
        if ( blocking_attacks.is_nonempty () ) 
        {
            /* Set boolean flag */
            black_has_blocking_sliding_pieces = true;

            /* Flood fill the blocking pieces along the block vectors, which will give legal attacks */
            blocking_attacks = blocking_attacks.flood_fill ( black_block_vectors );

            /* Sum rook attacks on open and semiopen files */
            straight_legal_attacks_open_diff -= ( blocking_attacks & black_straight_block_vectors & open_files ).popcount ();
            straight_legal_attacks_semiopen_diff -= ( blocking_attacks & black_straight_block_vectors & black_semiopen_files ).popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff -= ( blocking_attacks & black_center ).popcount ();

            /* Union restrictives attacked by diagonal pieces */
            restrictives_legally_attacked_by_black_diagonal_pieces |= restrictives & blocking_attacks;

            /* Get the number of diagonal restricted captures */
            diagonal_restricted_captures_diff -= ( blocking_attacks & black_diagonal_block_vectors & white_restrictives ).popcount ();

            /* Sum the legal captures on enemy straight pieces by diagonal pieces */
            diagonal_or_knight_captures_on_straight_diff -= ( blocking_attacks & black_diagonal_block_vectors & ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) ) ).popcount ();

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
            knight_attacks = knight_attacks.only_if ( !white_check_vectors.test ( pos ) ) & ~bb ( pcolor::white ) & white_check_vectors_dep_check_count;

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
            knight_attacks = knight_attacks.only_if ( !black_check_vectors.test ( pos ) ) & ~bb ( pcolor::black ) & black_check_vectors_dep_check_count;

            /* Sum mobility */
            black_mobility += knight_attacks.popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff -= ( knight_attacks & black_center ).popcount ();

            /* Sum the legal captures on enemy straight pieces by knights */
            diagonal_or_knight_captures_on_straight_diff += ( knight_attacks & ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) ) ).popcount ();

            /* Unset this bit */
            black_knights.reset ( pos );
        }



        /* Incorperate the number of knights into value */
        value += KNIGHT * ( bb ( pcolor::white, ptype::knight ).popcount () - bb ( pcolor::black, ptype::knight ).popcount () );

        /* Incorporate knights on initial cells into value */
        {
            const bitboard white_initial_knights = bb ( pcolor::white, ptype::bishop ) & white_knight_initial_cells;
            const bitboard black_initial_knights = bb ( pcolor::black, ptype::bishop ) & black_knight_initial_cells;
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
        /* Get the blocking and non-blocking pawns */
        const bitboard white_blocking_pawns     = bb ( pcolor::white, ptype::pawn ) &  white_block_vectors;
        const bitboard black_blocking_pawns     = bb ( pcolor::black, ptype::pawn ) &  black_block_vectors;
        const bitboard white_non_blocking_pawns = bb ( pcolor::white, ptype::pawn ) & ~white_block_vectors;
        const bitboard black_non_blocking_pawns = bb ( pcolor::black, ptype::pawn ) & ~black_block_vectors;

        /* Calculations for pawn pushes.
         * Non-blocking pawns can push without restriction.
         * Blocking pawns can push only if remaining within their block vector (which must be vertical).
         * If in check, ensure that the movements protected the king.
         */
        const bitboard white_pawn_pushes = white_non_blocking_pawns.pawn_push_n ( pp ) | ( white_blocking_pawns.pawn_push_n ( pp ) & white_straight_block_vectors ) & white_check_vectors_dep_check_count;
        const bitboard black_pawn_pushes = black_non_blocking_pawns.pawn_push_s ( pp ) | ( black_blocking_pawns.pawn_push_s ( pp ) & black_straight_block_vectors ) & black_check_vectors_dep_check_count;

        /* Get general pawn attacks */
        const bitboard white_pawn_attacks = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne ) | bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw );
        const bitboard black_pawn_attacks = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se ) | bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw );

        /* Get pawn captures (legal pawn attacks).
         * Non-blocking pawns can attack without restriction.
         * Blocking pawns can attack only if remaining within their block vector (which must be diagonal).
         * If in check, ensure that the movements protected the king.
         */
        const bitboard white_pawn_captures_e = ( white_non_blocking_pawns.pawn_attack ( diagonal_compass::ne, bb ( pcolor::black ) ) | ( white_blocking_pawns.pawn_attack ( diagonal_compass::ne, bb ( pcolor::black ) ) & white_diagonal_block_vectors ) ) & white_check_vectors_dep_check_count;
        const bitboard white_pawn_captures_w = ( white_non_blocking_pawns.pawn_attack ( diagonal_compass::nw, bb ( pcolor::black ) ) | ( white_blocking_pawns.pawn_attack ( diagonal_compass::nw, bb ( pcolor::black ) ) & white_diagonal_block_vectors ) ) & white_check_vectors_dep_check_count;
        const bitboard black_pawn_captures_e = ( black_non_blocking_pawns.pawn_attack ( diagonal_compass::se, bb ( pcolor::white ) ) | ( black_blocking_pawns.pawn_attack ( diagonal_compass::se, bb ( pcolor::white ) ) & black_diagonal_block_vectors ) ) & black_check_vectors_dep_check_count;
        const bitboard black_pawn_captures_w = ( black_non_blocking_pawns.pawn_attack ( diagonal_compass::sw, bb ( pcolor::white ) ) | ( black_blocking_pawns.pawn_attack ( diagonal_compass::sw, bb ( pcolor::white ) ) & black_diagonal_block_vectors ) ) & black_check_vectors_dep_check_count;



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
     * If the opposing player has blocking sliding pieces, then their defence union will be incomplete.
     * In this case, all king attacks must be individually validated to ensure they don't lead to check.
     */

    /* Scope the new bitboards */
    {
        /* Get all the king legal attacks (into empty space or an opposing piece).
         * The empty space must not be protected by the opponent, and the kings must not be left adjacent.
         * Note that some illegal attacks may be included here, if there are any opposing blocking sliding pieces.
         */
        bitboard white_king_attacks = white_king_span & ~black_king_span & ~bb ( pcolor::white ) & ~black_partial_defence_union;
        bitboard black_king_attacks = black_king_span & ~white_king_span & ~bb ( pcolor::black ) & ~white_partial_defence_union;

        /* If there are any possible white king attacks, detect if black defence union is incomplete */
        if ( white_king_attacks.is_nonempty () & black_has_blocking_sliding_pieces ) 
        {
            /* Create a temporary bitboard */
            bitboard white_king_attacks_temp = white_king_attacks;

            /* Loop through the white king attacks to validate they don't lead to check */
            do {
                /* Get the position of the next king attack then use the position to determine if it is defended by the opponent */
                unsigned pos = white_king_attacks_temp.trailing_zeros_nocheck ();
                white_king_attacks.reset_if ( pos, is_protected ( pcolor::black, pos ) );
                white_king_attacks_temp.reset ( pos );
            } while ( white_king_attacks_temp );
        }

        /* If there are any possible black king attacks, detect if white defence union is incomplete */
        if ( black_king_attacks.is_nonempty () & white_has_blocking_sliding_pieces ) 
        {
            /* Create a temporary bitboard */
            bitboard black_king_attacks_temp = black_king_attacks;

            /* Loop through the white king attacks to validate they don't lead to check */
            do {
                /* Get the position of the next king attack then use the position to determine if it is defended by the opponent */
                unsigned pos = black_king_attacks_temp.trailing_zeros_nocheck ();
                black_king_attacks.reset_if ( pos, is_protected ( pcolor::white, pos ) );
                black_king_attacks_temp.reset ( pos );
            } while ( black_king_attacks_temp );
        }

        /* Calculate king queen spans. Flood fill using pp and queen attack lookup as a propagator. */
        const bitboard white_king_queen_span = bb ( pcolor::white, ptype::king ).flood_fill ( bitboard::queen_attack_lookup ( white_king_pos ) & pp );
        const bitboard black_king_queen_span = bb ( pcolor::black, ptype::king ).flood_fill ( bitboard::queen_attack_lookup ( black_king_pos ) & pp );



        /* Sum mobility */
        white_mobility += white_king_attacks.popcount ();
        black_mobility += black_king_attacks.popcount ();

        /* Incorporate the king queen mobility into value */
        value += KING_QUEEN_MOBILITY * ( white_king_queen_span.popcount () - black_king_queen_span.popcount () );
    }



    /* CHECKMATE */

    /* Return maximum if one color has no mobility and the other does.
     * On a stalemate, currently continue normally.
     */
    if ( ( white_mobility == 0 ) & ( black_mobility != 0 ) ) return -CHECKMATE;
    if ( ( black_mobility == 0 ) & ( white_mobility != 0 ) ) return  CHECKMATE;



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
    return value;
}