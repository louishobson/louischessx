/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/chessboard_eval.cpp
 * 
 * Implementation of evaluation methods in include/chess/chessboard.h
 * 
 */



/* INCLUDES */
#include <louis_chessx/chessboard.h>



/* BOARD EVALUATION */



/** @name  get_check_info
 * 
 * @brief  Get information about the check state of a color's king
 * @param  pc: The color who's king we will look at
 * @return check_info_t
 */
chess::chessboard::check_info_t chess::chessboard::get_check_info ( pcolor pc ) const chess_validate_throw
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
    const int king_pos = king.trailing_zeros ();

    /* Get the positions of the opposing straight and diagonal pieces */
    const bitboard op_straight = bb ( npc, ptype::queen ) | bb ( npc, ptype::rook   );
    const bitboard op_diagonal = bb ( npc, ptype::queen ) | bb ( npc, ptype::bishop );

    /* Get the primary propagator.
     * Primary propagator of not opposing means that spans will overlook friendly pieces.
     * The secondary propagator will be universe.
     */
    const bitboard pp = ~opposing;
    const bitboard sp = ~bitboard {};



    /* KING, KNIGHTS AND PAWNS */

    /* Throw if kings are adjacent (this should never occur) */
#if CHESS_VALIDATE
    if ( bitboard::king_attack_lookup ( king_pos ) & bb ( npc, ptype::king ) ) throw std::runtime_error { "Adjacent king found in check_info ()." };
#endif

    /* Add checking knights */
    check_info.check_vectors |= bitboard::knight_attack_lookup ( king_pos ) & bb ( npc, ptype::knight );

    /* Switch depending on pc and add checking pawns */
    if ( pc == pcolor::white )
        check_info.check_vectors |= king.pawn_any_attack_n () & bb ( pcolor::black, ptype::pawn );
    else
        check_info.check_vectors |= king.pawn_any_attack_s () & bb ( pcolor::white, ptype::pawn );



    /* SLIDING PIECES */

    /* Iterate through the straight compass to see if those sliding pieces could be attacking */
    if ( bitboard::straight_attack_lookup ( king_pos ) & op_straight )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( const straight_compass dir : straight_compass_array )
    {
        /* Only continue if this is a possibly valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), king_pos ) & op_straight )
        {
            /* Span the king in the current direction */
            const bitboard king_span = king.rook_attack ( dir, pp, sp );

            /* Get the checking and blocking pieces */
            const bitboard checking = king_span & op_straight;
            const bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.check_vectors |= king_span.only_if ( checking.is_nonempty () & blocking.is_empty () );
            check_info.pin_vectors   |= king_span.only_if ( checking.is_nonempty () & blocking.is_singleton () );
        }
    }

    /* Iterate through the diagonal compass to see if those sliding pieces could be attacking */
    if ( bitboard::diagonal_attack_lookup ( king_pos ) & op_diagonal )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( const diagonal_compass dir : diagonal_compass_array )
    {
        /* Only continue if this is a possibly valid direction */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), king_pos ) & op_diagonal ) 
        {
            /* Span the king in the current direction */
            const bitboard king_span = king.bishop_attack ( dir, pp, sp );

            /* Get the checking and blocking pieces */
            const bitboard checking = king_span & op_diagonal;
            const bitboard blocking = king_span & friendly;

            /* Add check info */
            check_info.check_vectors |= king_span.only_if ( checking.is_nonempty () & blocking.is_empty () );
            check_info.pin_vectors   |= king_span.only_if ( checking.is_nonempty () & blocking.is_singleton () );
        }
    }



    /* SET REMAINING VARIABLES AND RETURN */

    /* Get the check count */
    check_info.check_count = ( check_info.check_vectors & bb ( npc ) ).popcount ();

    /* Set the check vectors along straight and diagonal paths */
    check_info.straight_check_vectors = check_info.check_vectors & bitboard::straight_attack_lookup ( king_pos );
    check_info.diagonal_check_vectors = check_info.check_vectors & bitboard::diagonal_attack_lookup ( king_pos );

    /* Set the pin vectors along straight and diagonal paths */
    check_info.straight_pin_vectors = check_info.pin_vectors & bitboard::straight_attack_lookup ( king_pos );
    check_info.diagonal_pin_vectors = check_info.pin_vectors & bitboard::diagonal_attack_lookup ( king_pos );

    /* Set check_vectors_dep_check_count */
    check_info.check_vectors_dep_check_count = check_info.check_vectors.all_if ( check_info.check_count == 0 ).only_if ( check_info.check_count < 2 );

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
bool chess::chessboard::is_protected ( pcolor pc, int pos ) const chess_validate_throw
{
    /* SETUP */

    /* Get a bitboard from pos */
    const bitboard pos_bb = singleton_bitboard ( pos );

    /* Get the positions of the friendly straight and diagonal pieces */
    const bitboard fr_straight = bb ( pc, ptype::queen ) | bb ( pc, ptype::rook );
    const bitboard fr_diagonal = bb ( pc, ptype::queen ) | bb ( pc, ptype::bishop );

    /* Get the propagators.
     * Primary propagator of not non-occupied will mean the span will stop at the first piece.
     * The secondary propagator of friendly pieces means the span will include a friendly piece if found.
     */
    const bitboard pp = ~bb ();
    const bitboard sp = bb ( pc );

    /* Get the adjacent open cells. These are the cells which don't contain an enemy piece or a friendly pawn, knight or king (and protection from a sliding piece could be blocked) */
    const bitboard adj_open_cells = bitboard::king_attack_lookup ( pos ) & ~bb ( other_color ( pc ) ) & ~bb ( pc, ptype::pawn ) & ~bb ( pc, ptype::knight ) & ~bb ( pc, ptype::king );



    /* KING, KNIGHTS AND PAWNS */

    /* Look for an adjacent king */
    if ( bitboard::king_attack_lookup ( pos ) & bb ( pc, ptype::king ) ) return true;

    /* Look for defending knights */
    if ( bitboard::knight_attack_lookup ( pos ) & bb ( pc, ptype::knight ) ) return true;

    /* Switch depending on pc and look for defending pawns */
    if ( pc == pcolor::white )
        { if ( pos_bb.pawn_any_attack_s () & bb ( pcolor::white, ptype::pawn ) ) return true; }
    else
        { if ( pos_bb.pawn_any_attack_n () & bb ( pcolor::black, ptype::pawn ) ) return true; }



    /* SLIDING PIECES */

    /* Iterate through the straight compass to see if those sliding pieces could be defending */
    if ( bitboard::straight_attack_lookup ( pos ).has_common ( adj_open_cells ) & bitboard::straight_attack_lookup ( pos ).has_common ( fr_straight ) )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( const straight_compass dir : straight_compass_array )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( adj_open_cells ) & bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( fr_straight ) )
            if ( pos_bb.rook_attack ( dir, pp, sp ) & fr_straight ) return true;      
    }

    /* Iterate through the diagonal compass to see if those sliding pieces could be defending */
    if ( bitboard::diagonal_attack_lookup ( pos ).has_common ( adj_open_cells ) & bitboard::diagonal_attack_lookup ( pos ).has_common ( fr_diagonal ) )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( const diagonal_compass dir : diagonal_compass_array )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected  */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( adj_open_cells ) & bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( fr_diagonal ) )
            if ( pos_bb.bishop_attack ( dir, pp, sp ) & fr_diagonal ) return true;
    }



    /* Return false */
    return false;    
}



/** @name  get_least_valuable_attacker
 * 
 * @brief  Takes a color and position and finds the least valuable piece attacking that color.
 * @param  pc: The color attacking.
 * @param  pos: The position being attacked.
 * @return A pair of ptype and position, no_piece and -1 if no attacker is found.
 */
std::pair<chess::ptype, int> chess::chessboard::get_least_valuable_attacker ( pcolor pc, int pos ) const chess_validate_throw
{
    /* SETUP */

    /* Get the check info */
    const check_info_t check_info = get_check_info ( pc );

    /* If pos is not part of check_vectors_dep_check_count, then no move will leave the king not in check, so no move is possible */
    if ( !check_info.check_vectors_dep_check_count.test ( pos ) ) return { ptype::no_piece, 0 };

    /* Get a bitboard from pos */
    const bitboard pos_bb = singleton_bitboard ( pos );

    /* Get the positions of the friendly straight and diagonal pieces. Disregard straight pieces on diagonal pin vectors and vice versa. */
    const bitboard fr_straight = ( bb ( pc, ptype::queen ) | bb ( pc, ptype::rook   ) ) & ~check_info.diagonal_pin_vectors;
    const bitboard fr_diagonal = ( bb ( pc, ptype::queen ) | bb ( pc, ptype::bishop ) ) & ~check_info.straight_pin_vectors;

    /* Get the propagators.
     * Primary propagator of not non-occupied will mean the span will stop at the first piece.
     * The secondary propagator of friendly pieces means the span will include a friendly piece if found.
     */
    const bitboard pp = ~bb ();
    const bitboard sp = bb ( pc );

    /* Get the adjacent open cells. These are the cells which don't contain an enemy piece or a friendly pawn, knight or king (and protection from a sliding piece could be blocked) */
    const bitboard adj_open_cells = bitboard::king_attack_lookup ( pos ) & ~bb ( other_color ( pc ) ) & ~bb ( pc, ptype::pawn ) & ~bb ( pc, ptype::knight ) & ~bb ( pc, ptype::king );



    /* PAWNS */

    {
        /* Switch depending on pc and look for attacking pawns */
        bitboard attackers = ( pc == pcolor::white ? pos_bb.pawn_any_attack_s () : pos_bb.pawn_any_attack_n () ) & bb ( pc, ptype::pawn );

        /* Remove pawns on straight pin vectors */
        attackers &= ~check_info.straight_pin_vectors;

        /* Remove the pawns on diagonal pin vectors, if they move off of their pin vector */
        attackers &= ~( attackers & check_info.diagonal_pin_vectors ).only_if_not ( check_info.diagonal_pin_vectors.test ( pos ) );

        /* If there are any pawns left, return one of them */
        if ( attackers ) return { ptype::pawn, attackers.trailing_zeros () };
    }



    /* KNIGHTS */

    {
        /* Get the attacking knights */
        bitboard attackers = bitboard::knight_attack_lookup ( pos ) & bb ( pc, ptype::knight );

        /* Remove those which are on pin vectors */
        attackers &= ~check_info.pin_vectors;

        /* If there are any knights left, return one of them */
        if ( attackers ) return { ptype::knight, attackers.trailing_zeros () };
    }



    /* SLIDING PIECES */

    /* If an attacking queen is found, the rest of the directions must be checked for lower value pieces.
     * Hence keep a variable storing the position of a queen, if found.
     */
    int attacking_queen_pos = -1;

    /* Iterate through diagonal sliding pieces first, since this way finding a bishop or rook can cause immediate return */

    /* Iterate through the diagonal compass to see if those sliding pieces could be defending */
    if ( bitboard::diagonal_attack_lookup ( pos ).has_common ( adj_open_cells ) & bitboard::diagonal_attack_lookup ( pos ).has_common ( fr_diagonal ) )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( const diagonal_compass dir : diagonal_compass_array )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected  */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( adj_open_cells ) & bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( fr_diagonal ) )
        {
            /* Get the attacker */
            bitboard attacker = pos_bb.bishop_attack ( dir, pp, sp ) & fr_diagonal;

            /* Get if there is an attacker, and it does not move off of a pin vector */
            if ( attacker && !( attacker.contains ( check_info.diagonal_pin_vectors ) && !check_info.diagonal_pin_vectors.test ( pos ) ) )
            {
                /* Get the position */
                const int attacker_pos = attacker.trailing_zeros ();

                /* If is a bishop, return now, else set attacking_queen_pos */
                if ( bb ( pc, ptype::bishop ).test ( attacker_pos ) ) return { ptype::bishop, attacker_pos }; else attacking_queen_pos = attacker_pos;
            }
        }
    }

    /* Iterate through the straight compass to see if those sliding pieces could be attacking */
    if ( bitboard::straight_attack_lookup ( pos ).has_common ( adj_open_cells ) & bitboard::straight_attack_lookup ( pos ).has_common ( fr_straight ) )
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 4
    for ( const straight_compass dir : straight_compass_array )
    {
        /* Only continue if this is a possibly valid direction, then return if is protected */
        if ( bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( adj_open_cells ) & bitboard::omnidir_attack_lookup ( static_cast<compass> ( dir ), pos ).has_common ( fr_straight ) )
        {
            /* Get the attackers */
            bitboard attacker = pos_bb.rook_attack ( dir, pp, sp ) & fr_straight;

            /* Get if there is an attacker, and it does not move off of a pin vector */
            if ( attacker && !( attacker.contains ( check_info.straight_pin_vectors ) && !check_info.straight_pin_vectors.test ( pos ) ) )
            {
                /* Get the position */
                const int attacker_pos = attacker.trailing_zeros ();

                /* If is a rook, return now, else set attacking_queen_pos */
                if ( bb ( pc, ptype::rook ).test ( attacker_pos ) ) return { ptype::rook, attacker_pos }; else attacking_queen_pos = attacker_pos;
            }
        }    
    }

    /* If an attacking queen was found, return that */
    if ( attacking_queen_pos != -1 ) return { ptype::queen, attacking_queen_pos };



    /* KING */

    /* Get if there is an adjacent king */
    if ( bitboard::king_attack_lookup ( pos ) & bb ( pc, ptype::king ) )
    {
        /* If pos is not protected by the other color, return the king as attacking */
        if ( !is_protected ( other_color ( pc ), pos ) ) return { ptype::king, bb ( pc, ptype::king ).trailing_zeros () };
    }



    /* NO PIECE */

    /* Return no_piece */
    return { ptype::no_piece, -1 };
}



/** @name  static_exchange_evaluation
 * 
 * @brief  Takes a color and position, and returns the possible material gain from the color attacking that position.
 * @param  pc: The color attacking.
 * @param  attacked_pos: The position being attacked.
 * @param  attacked_pt: The piece type to assume that's occupying pos. If no_piece, will be calculated.
 * @param  attacker_pos: The position of the first piece to attack attacked_pos.
 * @param  attacker_pt: The piece type that's first attacking attacked_pos. If no_piece, will be set to the least valuable attacker.
 * @param  prev_gain: A the gain from previous calls. Used internally, should be 0 initially.
 * @return An integer, 0 meaning no matierial gain, +/- meaning material gain or loss respectively.
 */
int chess::chessboard::static_exchange_evaluation ( pcolor pc, int attacked_pos, ptype attacked_pt, int attacker_pos, ptype attacker_pt, int prev_gain ) chess_validate_throw
{
    /* Create an array of material values for each ptype */
    constexpr std::array<int, 7> material_values { 100, 400, 400, 600, 1100, 10000, 0 };

    /* Get the attacked type, if required. Return prev_gain if there is no attacked piece. */
    if ( attacked_pt == ptype::no_piece ) attacked_pt = find_type ( other_color ( pc ), attacked_pos );
    if ( attacked_pt == ptype::no_piece ) return prev_gain;

    /* Get the speculative gain */
    const int spec_gain = prev_gain + material_values [ cast_penum ( attacked_pt ) ];

    /* Possibly cutoff */
    if ( std::max ( prev_gain, spec_gain ) < 0 ) return prev_gain;

    /* Get the least value piece of this color attacking pos.
     * If attacker_pos is set, then find the piece type at that position. Otherwise chose the least valuable attacker.
     * Return prev_gain if there is no such piece.
     */
    if ( attacker_pt == ptype::no_piece ) if ( attacked_pos != -1 ) attacker_pt = find_type ( pc, attacker_pos ); else
        std::tie ( attacker_pt, attacker_pos ) = get_least_valuable_attacker ( pc, attacked_pos );
    if ( attacker_pt == ptype::no_piece ) return prev_gain;

    /* Make the capture */
    get_bb ( pc              ).reset ( attacker_pos );
    get_bb ( pc, attacker_pt ).reset ( attacker_pos );
    get_bb ( pc              ).set   ( attacked_pos );
    get_bb ( pc, attacker_pt ).set   ( attacked_pos );
    get_bb ( other_color ( pc )              ).reset ( attacked_pos );
    get_bb ( other_color ( pc ), attacked_pt ).reset ( attacked_pos );

    /* Get the gain from see */
    const int gain = std::max ( prev_gain, -static_exchange_evaluation ( other_color ( pc ), attacked_pos, attacker_pt, -1, ptype::no_piece, -spec_gain ) );

    /* Unmake the capture */
    get_bb ( other_color ( pc )              ).set ( attacked_pos );
    get_bb ( other_color ( pc ), attacked_pt ).set ( attacked_pos );
    get_bb ( pc              ).reset ( attacked_pos );
    get_bb ( pc, attacker_pt ).reset ( attacked_pos );
    get_bb ( pc              ).set   ( attacker_pos );
    get_bb ( pc, attacker_pt ).set   ( attacker_pos );

    /* Return the gain */
    return gain;
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
    constexpr int QUEEN  { 1100 }; // 19
    constexpr int ROOK   {  600 }; // 10
    constexpr int BISHOP {  400 }; //  7
    constexpr int KNIGHT {  400 }; //  7
    constexpr int PAWN   {  100 }; //  2

    /* Pawns */
    constexpr int PAWN_GENERAL_ATTACKS                      {   1 }; // For every generally attacked cell
    constexpr int CENTER_PAWNS                              {  20 }; // For every pawn
    constexpr int PAWN_CENTER_GENERAL_ATTACKS               {  10 }; // For every generally attacked cell (in the center)
    constexpr int ISOLATED_PAWNS                            { -10 }; // For every pawn
    constexpr int ISOLATED_PAWNS_ON_SEMIOPEN_FILES          { -10 }; // For every pawn
    constexpr int DOUBLED_PAWNS                             {  -5 }; // Tripled pawns counts as -10 etc.
    constexpr int PAWN_GENERAL_ATTACKS_ADJ_OP_KING          {  20 }; // For every generally attacked cell
    constexpr int PHALANGA                                  {  20 }; // Tripple pawn counts as 40 etc.
    constexpr int BLOCKED_PASSED_PAWNS                      { -15 }; // For each blocked passed pawn
    constexpr int STRONG_SQUARES                            {  20 }; // For each strong square (one attacked by a friendly pawn and not an enemy pawn)
    constexpr int BACKWARD_PAWNS                            {  10 }; // For each pawn behind a strong square (see above)
    constexpr int PASSED_PAWNS_DISTANCE                     {   5 }; // For each passed pawn, for every square away from the 1st rank (since may be queened) 
    constexpr int LEGAL_ATTACKS_ON_PASSED_PAWN_TRAJECTORIES {   5 }; // For each attack

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
    constexpr int BISHOP_OR_KNIGHT_ON_STRONG_SQUARE             {  20 }; // For each piece

    /* Mobility and king queen mobility, for every move */
    constexpr int MOBILITY            {  1 }; // For every legal move
    constexpr int KING_QUEEN_MOBILITY { -2 }; // For every non-capture attack, pretending the king is a queen

    /* Castling */
    constexpr int CASTLE_MADE {  30 }; // If true
    constexpr int CASTLE_LOST { -60 }; // If true

    /* Other values */
    constexpr int KNIGHT_AND_QUEEN_EXIST               {  10 }; // If true
    constexpr int CENTER_LEGAL_ATTACKS_BY_RESTRICTIVES {  10 }; // For every attack (not including pawns or kings)
    constexpr int PINNED_PIECES                        { -20 }; // For each (friendly) piece

    /* Non-symmetrical values */
    constexpr int CHECKMATE           { 10000 }; // If on the enemy, -10000 if on self
    constexpr int KINGS_IN_OPPOSITION {    15 }; // If the kings are one off adjacent


    
    /* SETUP */

    /* Get the check info */
    const check_info_t white_check_info = get_check_info ( pcolor::white );
    const check_info_t black_check_info = get_check_info ( pcolor::black );

    /* Get king positions */
    const int white_king_pos = bb ( pcolor::white, ptype::king ).trailing_zeros ();
    const int black_king_pos = bb ( pcolor::black, ptype::king ).trailing_zeros ();

    /* Get the king spans */
    const bitboard white_king_span = bitboard::king_attack_lookup ( white_king_pos );
    const bitboard black_king_span = bitboard::king_attack_lookup ( black_king_pos );
    
    /* Set legalize_attacks. This is the intersection of not friendly pieces, not the enemy king, and check_vectors_dep_check_count. */
    const bitboard white_legalize_attacks = ~bb ( pcolor::white ) & ~bb ( pcolor::black, ptype::king ) & white_check_info.check_vectors_dep_check_count;
    const bitboard black_legalize_attacks = ~bb ( pcolor::black ) & ~bb ( pcolor::white, ptype::king ) & black_check_info.check_vectors_dep_check_count;

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

    /* Get the passed pawns */
    const bitboard white_passed_pawns = bb ( pcolor::white, ptype::pawn ) & ~white_pawn_rear_span & black_semiopen_files & black_semiopen_files.shift ( compass::e ) & black_semiopen_files.shift ( compass::w );
    const bitboard black_passed_pawns = bb ( pcolor::black, ptype::pawn ) & ~black_pawn_rear_span & white_semiopen_files & white_semiopen_files.shift ( compass::e ) & white_semiopen_files.shift ( compass::w );

    /* Get the cells behind passed pawns. 
     * This is the span between the passed pawns and the next piece back, including the next piece back but not the pawn.
     */ 
    const bitboard white_behind_passed_pawns = white_passed_pawns.span ( compass::s, pp, ~bitboard {} );
    const bitboard black_behind_passed_pawns = black_passed_pawns.span ( compass::n, pp, ~bitboard {} );

    /* Get the cells in front of passed pawns.
     * This is the span between the passed pawns and the opposite end of the board, not including the pawn.
     */
    const bitboard white_passed_pawn_trajectories = white_passed_pawns.span ( compass::n );
    const bitboard black_passed_pawn_trajectories = black_passed_pawns.span ( compass::s );



    /* ACCUMULATORS */

    /* The value of the evaluation function */
    int value = 0;

    /* Mobilities */
    int white_mobility = 0, black_mobility = 0;

    /* Partial defence unions. Gives cells which are protected by friendly pieces.
     * This is found from the union of all general attacks and can be used to determine opposing king's possible moves.
     * However, pinned sliding pieces' general attacks are not included in this union (due to complexity to calculate), hence 'partial'.
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

    /* Accumulate attacks on passed pawn trajectories */
    int legal_attacks_on_passed_pawn_trajectories_diff = 0;



    /* NON-PINNED SLIDING PIECES */

    /* Deal with non-pinned sliding pieces of both colors in one go.
     * If in double check, check_vectors_dep_check_count will remove all attacks the attacks.
     */ 

    /* Scope new bitboards */
    {        
        /* Set straight and diagonal pieces */
        const bitboard white_straight_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook   ) ) & ~white_check_info.pin_vectors;
        const bitboard white_diagonal_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::bishop ) ) & ~white_check_info.pin_vectors;
        const bitboard black_straight_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook   ) ) & ~black_check_info.pin_vectors;
        const bitboard black_diagonal_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::bishop ) ) & ~black_check_info.pin_vectors;

        /* Start compasses */
        straight_compass straight_dir;
        diagonal_compass diagonal_dir;

        /* Iterate through the compass to get all queen, rook and bishop attacks */
        #pragma clang loop unroll ( full )
        #pragma GCC unroll 4
        for ( int i = 0; i < 4; ++i )
        {
            /* Get the compasses */
            straight_dir = straight_compass_array [ i ];
            diagonal_dir = diagonal_compass_array [ i ];

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

            /* Sum the legal attacks on passed pawn trajectories */
            legal_attacks_on_passed_pawn_trajectories_diff += ( white_straight_attacks & black_passed_pawn_trajectories ).popcount () + ( white_diagonal_attacks & black_passed_pawn_trajectories ).popcount ();
            legal_attacks_on_passed_pawn_trajectories_diff -= ( black_straight_attacks & white_passed_pawn_trajectories ).popcount () + ( black_diagonal_attacks & white_passed_pawn_trajectories ).popcount ();
        }
    }



    /* PINNED SLIDING PIECES */

    /* Pinned pieces are handelled separately for each color.
     * Only if that color is not in check will they be considered.
     * The flood spans must be calculated separately for straight and diagonal, since otherwise the flood could spill onto adjacent pin vectors.
     */

    /* White pinned pieces */
    if ( white_check_info.pin_vectors.is_nonempty () & white_check_info.check_count == 0 )
    {
        /* Get the straight and diagonal pinned pieces which can move.
         * Hence the straight pinned pieces are all on straight pin vectors, and diagonal pinned pieces are all on diagonal pin vectors.
         */
        const bitboard straight_pinned_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook   ) ) & white_check_info.straight_pin_vectors;
        const bitboard diagonal_pinned_pieces = ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::bishop ) ) & white_check_info.diagonal_pin_vectors;

        /* Initially set pinned attacks to empty */
        bitboard straight_pinned_attacks, diagonal_pinned_attacks;

        /* Flood span straight and diagonal (but only if there are pieces to flood) */
        if ( straight_pinned_pieces ) straight_pinned_attacks = straight_pinned_pieces.straight_flood_span ( white_check_info.straight_pin_vectors );
        if ( diagonal_pinned_pieces ) diagonal_pinned_attacks = diagonal_pinned_pieces.diagonal_flood_span ( white_check_info.diagonal_pin_vectors );

        /* Get the union of pinned attacks */
        const bitboard pinned_attacks = straight_pinned_attacks | diagonal_pinned_attacks;

        /* Only continue if there were appropriate pinned attacks */
        if ( pinned_attacks )
        {
            /* Union defence (at this point the defence union becomes partial) */
            white_partial_defence_union |= pinned_attacks | bb ( pcolor::white, ptype::king );

            /* Sum mobility */
            white_mobility += pinned_attacks.popcount ();

            /* Sum rook attacks on open and semiopen files */
            straight_legal_attacks_open_diff += ( straight_pinned_attacks & open_files ).popcount ();
            straight_legal_attacks_semiopen_diff += ( straight_pinned_attacks & white_semiopen_files ).popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff += ( pinned_attacks & white_center ).popcount ();

            /* Union restrictives attacked by diagonal pieces */
            restrictives_legally_attacked_by_white_diagonal_pieces |= restrictives & diagonal_pinned_attacks;

            /* Get the number of diagonal restricted captures */
            diagonal_restricted_captures_diff += ( diagonal_pinned_attacks & black_restrictives ).popcount ();

            /* Sum the legal captures on enemy straight pieces by diagonal pieces */
            diagonal_or_knight_captures_on_straight_diff += ( diagonal_pinned_attacks & ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook ) ) ).popcount ();
        
            /* Sum the legal attacks on passed pawn trajectories */
            legal_attacks_on_passed_pawn_trajectories_diff += ( pinned_attacks & black_passed_pawn_trajectories ).popcount ();
        }
    }

    /* Black pinned pieces */
    if ( black_check_info.pin_vectors.is_nonempty () & black_check_info.check_count == 0 )
    {
        /* Get the straight and diagonal pinned pieces which can move.
         * Hence the straight pinned pieces are all on straight pin vectors, and diagonal pinned pieces are all on diagonal pin vectors.
         */
        const bitboard straight_pinned_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook   ) ) & black_check_info.straight_pin_vectors;
        const bitboard diagonal_pinned_pieces = ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::bishop ) ) & black_check_info.diagonal_pin_vectors;

        /* Initially set pinned attacks to empty */
        bitboard straight_pinned_attacks, diagonal_pinned_attacks;

        /* Flood span straight and diagonal (but only if there are pieces to flood) */
        if ( straight_pinned_pieces ) straight_pinned_attacks = straight_pinned_pieces.straight_flood_span ( black_check_info.straight_pin_vectors );
        if ( diagonal_pinned_pieces ) diagonal_pinned_attacks = diagonal_pinned_pieces.diagonal_flood_span ( black_check_info.diagonal_pin_vectors );

        /* Get the union of pinned attacks */
        const bitboard pinned_attacks = straight_pinned_attacks | diagonal_pinned_attacks;

        /* Only continue if there were appropriate pinned attacks */
        if ( pinned_attacks )
        {
            /* Union defence (at this point the defence union becomes partial) */
            black_partial_defence_union |= pinned_attacks | bb ( pcolor::black, ptype::king );

            /* Sum mobility */
            black_mobility += pinned_attacks.popcount ();

            /* Sum rook attacks on open and semiopen files */
            straight_legal_attacks_open_diff -= ( straight_pinned_attacks & open_files ).popcount ();
            straight_legal_attacks_semiopen_diff -= ( straight_pinned_attacks & black_semiopen_files ).popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff -= ( pinned_attacks & black_center ).popcount ();

            /* Union restrictives attacked by diagonal pieces */
            restrictives_legally_attacked_by_black_diagonal_pieces |= restrictives & diagonal_pinned_attacks;

            /* Get the number of diagonal restricted captures */
            diagonal_restricted_captures_diff -= ( diagonal_pinned_attacks & white_restrictives ).popcount ();

            /* Sum the legal captures on enemy straight pieces by diagonal pieces */
            diagonal_or_knight_captures_on_straight_diff -= ( diagonal_pinned_attacks & ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) ) ).popcount ();

            /* Sum the legal attacks on passed pawn trajectories */
            legal_attacks_on_passed_pawn_trajectories_diff -= ( pinned_attacks & white_passed_pawn_trajectories ).popcount ();
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
        /* Iterate through white knights to get attacks.
         * If in double check, white_check_info.check_vectors_dep_check_count will remove all moves. 
         */
        for ( bitboard white_knights = bb ( pcolor::white, ptype::knight ); white_knights; )
        {
            /* Get the position of the next knight and its general attacks */
            const int pos = white_knights.trailing_zeros (); white_knights.reset ( pos );
            bitboard knight_attacks = bitboard::knight_attack_lookup ( pos );

            /* Union defence */
            white_partial_defence_union |= knight_attacks;

            /* Find legal knight attacks. Note that a pinned knight cannot move along its pin vector, hence cannot move at all. */
            knight_attacks = knight_attacks.only_if ( !white_check_info.pin_vectors.test ( pos ) ) & white_legalize_attacks;

            /* Sum mobility */
            white_mobility += knight_attacks.popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff += ( knight_attacks & white_center ).popcount ();

            /* Sum the legal captures on enemy straight pieces by knights */
            diagonal_or_knight_captures_on_straight_diff += ( knight_attacks & ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook ) ) ).popcount ();         

            /* Sum the legal attacks on passed pawn trajectories */
            legal_attacks_on_passed_pawn_trajectories_diff += ( knight_attacks & black_passed_pawn_trajectories ).popcount ();   
        }

        /* Iterate through black knights to get attacks */
        for ( bitboard black_knights = bb ( pcolor::black, ptype::knight ); black_knights; )
        {
            /* Get the position of the next knight and its general attacks */
            const int pos = black_knights.trailing_zeros (); black_knights.reset ( pos );
            bitboard knight_attacks = bitboard::knight_attack_lookup ( pos );

            /* Union defence */
            black_partial_defence_union |= knight_attacks;

            /* Find legal knight attacks. Note that a pinned knight cannot move along its pin vector, hence cannot move at all. */
            knight_attacks = knight_attacks.only_if ( !black_check_info.pin_vectors.test ( pos ) ) & black_legalize_attacks;

            /* Sum mobility */
            black_mobility += knight_attacks.popcount ();

            /* Sum attacks on the center */
            center_legal_attacks_by_restrictives_diff -= ( knight_attacks & black_center ).popcount ();

            /* Sum the legal captures on enemy straight pieces by knights */
            diagonal_or_knight_captures_on_straight_diff -= ( knight_attacks & ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) ) ).popcount ();

            /* Sum the legal attacks on passed pawn trajectories */
            legal_attacks_on_passed_pawn_trajectories_diff -= ( knight_attacks & white_passed_pawn_trajectories ).popcount ();
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

    /* We can handle both colors, pinned and non-pinned pawns simultaneously, since pawn moves are more simple than sliding pieces and knights.
     * Non-pinned pawns can have their moves calculated normally, but pinned pawns must first be checked as to whether they will move along their pin vector.
     * There is no if-statement for double check, only check_vectors_dep_check_count is used to mask out all moves if in double check.
     * This is because double check is quite unlikely, and pawn move calculations are quite cheap anyway.
     */

    /* Scope the new bitboards */
    {
        /* Get the non-pinned pawns */
        const bitboard white_non_pinned_pawns = bb ( pcolor::white, ptype::pawn ) & ~white_check_info.pin_vectors;
        const bitboard black_non_pinned_pawns = bb ( pcolor::black, ptype::pawn ) & ~black_check_info.pin_vectors;
        
        /* Get the straight and diagonal pinned pawns */
        const bitboard white_straight_pinned_pawns = bb ( pcolor::white, ptype::pawn ) & white_check_info.straight_pin_vectors;
        const bitboard white_diagonal_pinned_pawns = bb ( pcolor::white, ptype::pawn ) & white_check_info.diagonal_pin_vectors;
        const bitboard black_straight_pinned_pawns = bb ( pcolor::black, ptype::pawn ) & black_check_info.straight_pin_vectors;
        const bitboard black_diagonal_pinned_pawns = bb ( pcolor::black, ptype::pawn ) & black_check_info.diagonal_pin_vectors;

        /* Calculations for pawn pushes.
         * Non-pinned pawns can push without restriction.
         * Pinned pawns can push only if they started and ended within a straight pin vector.
         * If in check, ensure that the movements protected the king.
         */
        const bitboard white_pawn_pushes = ( white_non_pinned_pawns.pawn_push_n ( pp ) | ( white_straight_pinned_pawns.pawn_push_n ( pp ) & white_check_info.straight_pin_vectors ) ) & white_check_info.check_vectors_dep_check_count;
        const bitboard black_pawn_pushes = ( black_non_pinned_pawns.pawn_push_s ( pp ) | ( black_straight_pinned_pawns.pawn_push_s ( pp ) & black_check_info.straight_pin_vectors ) ) & black_check_info.check_vectors_dep_check_count;

        /* Get general pawn attacks */
        const bitboard white_pawn_attacks = bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne ) | bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw );
        const bitboard black_pawn_attacks = bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se ) | bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw );

        /* Get the strong squares.
         * These are the squares attacked by a friendly pawn, but not by an enemy pawn.
         */
        const bitboard white_strong_squares = white_pawn_attacks & ~black_pawn_attacks;
        const bitboard black_strong_squares = black_pawn_attacks & ~white_pawn_attacks;

        /* Get pawn captures (legal pawn attacks).
         * Non-pinned pawns can attack without restriction.
         * Pinned pawns can attack only if they started and ended within a diagonal pin vector.
         * If in check, ensure that the movements protected the king.
         * En passant will be added later
         */
        bitboard white_pawn_captures_e = ( white_non_pinned_pawns.pawn_attack ( diagonal_compass::ne ) | ( white_diagonal_pinned_pawns.pawn_attack ( diagonal_compass::ne ) & white_check_info.diagonal_pin_vectors ) ) & bb ( pcolor::black ) & white_legalize_attacks;
        bitboard white_pawn_captures_w = ( white_non_pinned_pawns.pawn_attack ( diagonal_compass::nw ) | ( white_diagonal_pinned_pawns.pawn_attack ( diagonal_compass::nw ) & white_check_info.diagonal_pin_vectors ) ) & bb ( pcolor::black ) & white_legalize_attacks;
        bitboard black_pawn_captures_e = ( black_non_pinned_pawns.pawn_attack ( diagonal_compass::se ) | ( black_diagonal_pinned_pawns.pawn_attack ( diagonal_compass::se ) & black_check_info.diagonal_pin_vectors ) ) & bb ( pcolor::white ) & black_legalize_attacks;
        bitboard black_pawn_captures_w = ( black_non_pinned_pawns.pawn_attack ( diagonal_compass::sw ) | ( black_diagonal_pinned_pawns.pawn_attack ( diagonal_compass::sw ) & black_check_info.diagonal_pin_vectors ) ) & bb ( pcolor::white ) & black_legalize_attacks;

        /* Save the en passant target square */
        const int en_passant_target = aux_info.en_passant_target;

        /* Add white en passant captures if they don't lead to check */
        if ( bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::ne ).only_if ( aux_info.en_passant_color == pcolor::white ).test ( en_passant_target ) )
        {
            make_move_internal ( move_t { pcolor::white, ptype::pawn, ptype::pawn, ptype::no_piece, en_passant_target - 9, en_passant_target } );
            if ( !is_in_check ( pc ) ) white_pawn_captures_e.set ( en_passant_target );
            unmake_move_internal ();
        } if ( bb ( pcolor::white, ptype::pawn ).pawn_attack ( diagonal_compass::nw ).only_if ( aux_info.en_passant_color == pcolor::white ).test ( en_passant_target ) )
        {
            make_move_internal ( move_t { pcolor::white, ptype::pawn, ptype::pawn, ptype::no_piece, en_passant_target - 7, en_passant_target } );
            if ( !is_in_check ( pc ) ) white_pawn_captures_w.set ( en_passant_target );
            unmake_move_internal ();
        }
        
        /* Add black en passant captures if they don't lead to check */
        if ( bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::se ).only_if ( aux_info.en_passant_color == pcolor::black ).test ( en_passant_target ) )
        {
            make_move_internal ( move_t { pcolor::black, ptype::pawn, ptype::pawn, ptype::no_piece, en_passant_target + 7, en_passant_target } );
            if ( !is_in_check ( pc ) ) black_pawn_captures_e.set ( en_passant_target );
            unmake_move_internal ();
        } if ( bb ( pcolor::black, ptype::pawn ).pawn_attack ( diagonal_compass::sw ).only_if ( aux_info.en_passant_color == pcolor::black ).test ( en_passant_target ) )
        {
            make_move_internal ( move_t { pcolor::black, ptype::pawn, ptype::pawn, ptype::no_piece, en_passant_target + 9, en_passant_target } );
            if ( !is_in_check ( pc ) ) black_pawn_captures_w.set ( en_passant_target );
            unmake_move_internal ();
        }



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

        /* Incorporate strong squares into value */
        value += STRONG_SQUARES * ( white_strong_squares.popcount () - black_strong_squares.popcount () );

        /* Sum the legal attacks on passed pawn trajectories.
         * Use pawn general attacks, since legal captures require there to be a piece present.
         */
        legal_attacks_on_passed_pawn_trajectories_diff += ( white_pawn_attacks & black_passed_pawn_trajectories ).popcount () - ( black_pawn_attacks & white_passed_pawn_trajectories ).popcount ();

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
            const bitboard white_blocked_passed_pawns = white_behind_passed_pawns.shift ( compass::n ).shift ( compass::n ) & bb ( pcolor::black );
            const bitboard black_blocked_passed_pawns = black_behind_passed_pawns.shift ( compass::s ).shift ( compass::s ) & bb ( pcolor::white );
            value += BLOCKED_PASSED_PAWNS * ( white_blocked_passed_pawns.popcount () - black_blocked_passed_pawns.popcount () );
        }

        /* Incorporate backwards pawns into value */
        {
            const bitboard white_backward_pawns = white_strong_squares.shift ( compass::s ) & bb ( pcolor::white, ptype::pawn );
            const bitboard black_backward_pawns = black_strong_squares.shift ( compass::n ) & bb ( pcolor::black, ptype::pawn );
            value += BACKWARD_PAWNS * ( white_backward_pawns.popcount () - black_backward_pawns.popcount () );
        }

        /* Incorporate bishops or knights on strong squares into value */
        {
            const bitboard white_bishop_or_knight_strong_squares = white_strong_squares & ( bb ( pcolor::white, ptype::bishop ) | bb ( pcolor::white, ptype::knight ) );
            const bitboard black_bishop_or_knight_strong_squares = black_strong_squares & ( bb ( pcolor::black, ptype::bishop ) | bb ( pcolor::black, ptype::knight ) );
            value += BISHOP_OR_KNIGHT_ON_STRONG_SQUARE * ( white_bishop_or_knight_strong_squares.popcount () - black_bishop_or_knight_strong_squares.popcount () );
        }

        /* Incorporate passed pawns into value */
        {
            const bitboard white_passed_pawns_distance = white_passed_pawns.fill ( compass::s );
            const bitboard black_passed_pawns_distance = black_passed_pawns.fill ( compass::n );
            value += PASSED_PAWNS_DISTANCE * ( white_passed_pawns_distance.popcount () - black_passed_pawns_distance.popcount () );
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
         * Note that some illegal attacks may be included here, if there are any opposing pinned sliding pieces.
         */
        bitboard white_king_attacks = white_king_span & ~black_king_span & ~bb ( pcolor::white ) & ~black_partial_defence_union;
        bitboard black_king_attacks = black_king_span & ~white_king_span & ~bb ( pcolor::black ) & ~white_partial_defence_union;

        /* Validate the remaining white king moves */
        if ( white_king_attacks ) 
        {
            /* Temporarily unset the king */
            get_bb ( pcolor::white ).reset              ( white_king_pos );
            get_bb ( pcolor::white, ptype::king ).reset ( white_king_pos );

            /* Loop through the white king attacks to validate they don't lead to check */
            for ( bitboard white_king_attacks_temp = white_king_attacks; white_king_attacks_temp; )
            {
                /* Get the position of the next king attack then use the position to determine if it is defended by the opponent */
                int pos = white_king_attacks_temp.trailing_zeros ();
                white_king_attacks.reset_if ( pos, is_protected ( pcolor::black, pos ) );
                white_king_attacks_temp.reset ( pos );
            }

            /* Reset the king */
            get_bb ( pcolor::white ).set              ( white_king_pos );
            get_bb ( pcolor::white, ptype::king ).set ( white_king_pos );
        }

        /* Validate the remaining black king moves */
        if ( black_king_attacks ) 
        {
            /* Temporarily unset the king */
            get_bb ( pcolor::black ).reset              ( black_king_pos );
            get_bb ( pcolor::black, ptype::king ).reset ( black_king_pos );

            /* Loop through the white king attacks to validate they don't lead to check */
            for ( bitboard black_king_attacks_temp = black_king_attacks; black_king_attacks_temp; )
            { 
                /* Get the position of the next king attack then use the position to determine if it is defended by the opponent */
                int pos = black_king_attacks_temp.trailing_zeros ();
                black_king_attacks.reset_if ( pos, is_protected ( pcolor::white, pos ) );
                black_king_attacks_temp.reset ( pos );
            }

            /* Reset the king */
            get_bb ( pcolor::black ).set              ( black_king_pos );
            get_bb ( pcolor::black, ptype::king ).set ( black_king_pos );
        }

        /* Calculate king queen fill. Flood fill using pp and queen attack lookup as a propagator. */
        const bitboard white_king_queen_fill = bb ( pcolor::white, ptype::king ).straight_flood_fill ( bitboard::straight_attack_lookup ( white_king_pos ) & pp )
                                             | bb ( pcolor::white, ptype::king ).diagonal_flood_fill ( bitboard::diagonal_attack_lookup ( white_king_pos ) & pp );
        const bitboard black_king_queen_fill = bb ( pcolor::black, ptype::king ).straight_flood_fill ( bitboard::straight_attack_lookup ( black_king_pos ) & pp )
                                             | bb ( pcolor::black, ptype::king ).diagonal_flood_fill ( bitboard::diagonal_attack_lookup ( black_king_pos ) & pp );



        /* Sum mobility */
        white_mobility += white_king_attacks.popcount () + can_kingside_castle ( pcolor::white, white_check_info ) + can_queenside_castle ( pcolor::white, white_check_info );
        black_mobility += black_king_attacks.popcount () + can_kingside_castle ( pcolor::black, black_check_info ) + can_queenside_castle ( pcolor::black, black_check_info ); 

        /* Incorporate the king queen mobility into value.
         * It does not matter that we are using the fills instead of spans, since the fact both the fills include their kings will cancel out.
         */
        value += KING_QUEEN_MOBILITY * ( white_king_queen_fill.popcount () - black_king_queen_fill.popcount () );

        /* Sum the legal attacks on passed pawn trajectories */
        legal_attacks_on_passed_pawn_trajectories_diff += ( white_king_span & black_passed_pawn_trajectories ).popcount () - ( black_king_span & white_passed_pawn_trajectories ).popcount ();
    }

    /* CHECKMATE AND STALEMATE */

    /* If one color has no mobility and the other does, return maxima/minima or 0, depending on check count */
    if ( ( white_mobility == 0 ) & ( black_mobility != 0 ) ) if ( white_check_info.check_count ) return ( pc == pcolor::white ? -CHECKMATE :  CHECKMATE ); else return 0;
    if ( ( black_mobility == 0 ) & ( white_mobility != 0 ) ) if ( black_check_info.check_count ) return ( pc == pcolor::white ?  CHECKMATE : -CHECKMATE ); else return 0;

    /* If neither color has any mobility, return 0 */
    if ( white_mobility == 0 && black_mobility == 0 ) return 0;



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

    /* Pinned pieces */
    {
        const bitboard white_pinned_pieces = white_check_info.pin_vectors & bb ( pcolor::white );
        const bitboard black_pinned_pieces = black_check_info.pin_vectors & bb ( pcolor::black );
        value += PINNED_PIECES * ( white_pinned_pieces.popcount () - black_pinned_pieces.popcount () );
    }

    /* Attacks on passed pawn trajectories */
    value += LEGAL_ATTACKS_ON_PASSED_PAWN_TRAJECTORIES * legal_attacks_on_passed_pawn_trajectories_diff;

    /* Kings in opposition */
    value += KINGS_IN_OPPOSITION * ( white_king_span & black_king_span ).is_nonempty () * ( pc == pcolor::white ? +1 : -1 );



    /* Return the value */
    return ( pc == pcolor::white ? value : -value );
}