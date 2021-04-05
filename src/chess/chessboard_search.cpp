/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * src/chess/chessboard_search.cpp
 * 
 * Implementation of search methods in include/chess/chessboard.h
 * 
 */



/* INCLUDES */
#include <chess/chessboard.h>



/* TTABLE PURGING */

/** @name  purge_ttable
 * 
 * @brief  Take a transposition table, and remove entries which are no longer reachable from the current board state.
 * @param  ttable: The transposition table to erase elements from.
 * @return A new ttable with unreachable positions erased.
 */
chess::chessboard::ab_ttable_t chess::chessboard::purge_ttable ( ab_ttable_t ttable ) const
{
    /* Erase elements */
    std::erase_if ( ttable, [ this ] ( const ab_ttable_t::value_type& entry )
    {
        /* Erase if the count of any piece type is not the same */
        for ( pcolor pc : { pcolor::white, pcolor::black } ) for ( ptype pt : ptype_inc_value ) if ( entry.first.bb ( pc, pt ).popcount () > bb ( pc, pt ).popcount () ) return true;

        /* Loop through the pawns in the ttable state, which are not present in the current state */
        for ( bitboard moved_pawns = entry.first.bb ( pcolor::white, ptype::pawn ) & ~bb ( pcolor::white, ptype::pawn ); moved_pawns; )
        {
            /* Get the next pawn */
            int pos = moved_pawns.trailing_zeros (); moved_pawns.reset ( pos );

            /* If there is no pawn in the current state, which can feasibly reach pos and is not already in the ttable state, then purge this ttable state */
            if ( !( bitboard::pawn_pyramid_s_lookup ( pos ) & bb ( pcolor::white, ptype::pawn ) & ~entry.first.bb ( pcolor::white, ptype::pawn ) ) ) return true;
        }

        /* Do the same for black */
        for ( bitboard moved_pawns = entry.first.bb ( pcolor::black, ptype::pawn ) & ~bb ( pcolor::black, ptype::pawn ); moved_pawns; )
        {
            int pos = moved_pawns.trailing_zeros (); moved_pawns.reset ( pos );
            if ( !( bitboard::pawn_pyramid_n_lookup ( pos ) & bb ( pcolor::black, ptype::pawn ) & ~entry.first.bb ( pcolor::black, ptype::pawn ) ) ) return true;
        }

        /* Do not delete */
        return false;
    } );

    /* Return ttable */
    return ttable;
}



/* ALPHA BETA SEARCH */



/** @name  alpha_beta_search
 * 
 * @brief  Set up and apply the alpha-beta search.
 * @param  pc: The color whose move it is next.
 * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
 * @param  best_only: If true, the search will be optimised as only the best move is returned.
 * @param  ttable: The transposition table to use for the search. Empty by default.
 * @param  end_flag: An atomic boolean, which when set to true, will end the search. Can be unspecified.
 * @param  end_point: A time point at which the search will be automatically stopped. Never by default.
 * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
 * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
 * @return ab_result_t
 */
chess::chessboard::ab_result_t chess::chessboard::alpha_beta_search ( const pcolor pc, const int depth, const bool best_only, ab_ttable_t ttable, const std::atomic_bool& end_flag, const chess_clock::time_point end_point, const int alpha, const int beta )
{
    /* Allocate ab_working */
    ab_working.reset ( new ab_working_t );

    /* Allocate excess memory  */ 
    ab_working->move_sets.resize ( 32 ); 
    ab_working->killer_moves.resize ( 32 );
    
    /* Reserve excess memory for root moves */
    ab_working->root_moves.reserve ( 32 );

    /* Move over ttable */
    ab_working->ttable = std::move ( ttable );

    /* Reset counters */
    ab_working->sum_q_depth = ab_working->sum_moves = ab_working->sum_q_moves = ab_working->num_nodes = ab_working->num_q_nodes = 0;

    /* Call and time the internal method */
    const auto t0 = chess_clock::now ();
    alpha_beta_search_internal ( pc, depth, best_only, end_flag, end_point, alpha, beta );
    const auto t1 = chess_clock::now ();

    /* Create the ab result struct */
    ab_result_t ab_result;

    /* Set the values */
    ab_result.moves       = std::move ( ab_working->root_moves );
    ab_result.depth       = depth; 
    ab_result.num_nodes   = ab_working->num_nodes;
    ab_result.num_q_nodes = ab_working->num_q_nodes;
    ab_result.av_q_depth  = ab_working->sum_q_depth / static_cast<double> ( ab_working->num_q_nodes );
    ab_result.av_moves    = ab_working->sum_moves   / static_cast<double> ( ab_working->num_nodes   );
    ab_result.av_q_moves  = ab_working->sum_q_moves / static_cast<double> ( ab_working->num_q_nodes );
    ab_result.incomplete  = end_flag || chess_clock::now () > end_point;
    ab_result.duration    = t1 - t0;
    ab_result.ttable      = std::move ( ab_working->ttable );

    /* Delete the working values */
    ab_working.reset ( nullptr );

    /* If there are no possible moves, return now */
    if ( ab_result.moves.empty () ) return ab_result;

    /* Order the moves */
    std::sort ( ab_result.moves.begin (), ab_result.moves.end (), [] ( const auto& lhs, const auto& rhs ) { return lhs.second > rhs.second; } );

    /* Resize the moves list to 1 if best_only is set */
    if ( best_only ) ab_result.moves.resize ( 1 );

    /* Set the check, checkmate stalemate and draw booleans for each move */
    for ( auto& move : ab_result.moves )
    {
        /* Make the move */
        make_move_internal ( move.first ); 

        /* Get the check info and mobility for npc */
        const check_info_t npc_check_info = get_check_info ( other_color ( pc ) );
        const bool npc_has_mobility       = has_mobility   ( other_color ( pc ), npc_check_info );

        /* Get the check, checkmate, stalemate and draw booleans */
        move.first.check     =  npc_check_info.check_count;
        move.first.checkmate =  npc_check_info.check_count && !npc_has_mobility;
        move.first.stalemate = !npc_check_info.check_count && !npc_has_mobility;
        move.first.draw      = is_draw_state ();

        /* Unmake the move */
        unmake_move_internal ();
    }

    /* Set failed low and failed high flags */
    ab_result.failed_low  = ab_result.moves.back  ().second <= alpha;
    ab_result.failed_high = ab_result.moves.front ().second >= beta;
    
    /* Return the alpha beta result */
    return ab_result;
}



/* ITERATIVE DEEPENING */



/** @name  alpha_beta_iterative_deepening
 * 
 * @brief  Apply an alpha-beta search over a range of depths.
 *         Specifying an end point could to an early return rather than starting later searches.
 * @param  pc: The color whose move it is next.
 * @param  depths: A list of depth values to search.
 * @param  best_only: If true, the search will be optimised as only the best move is returned.
 * @param  ttable: The transposition table to use for the search. Empty by default.
 * @param  end_flag: An atomic boolean, which when set to true, will end the search. Can be unspecified.
 * @param  end_point: A time point at which the search will be automatically stopped. Never by default.
 * @param  finish_first: If true, always wait for the lowest depth search to finish, regardless of end_point or end_flag. True by default.
 * @return ab_result_t
 */
chess::chessboard::ab_result_t chess::chessboard::alpha_beta_iterative_deepening ( const pcolor pc, const std::vector<int>& depths, const bool best_only, ab_ttable_t ttable, const std::atomic_bool& end_flag, const chess_clock::time_point end_point, const bool finish_first )
{
    /* The result of the highest depth complete search */
    ab_result_t ab_result;

    /* Set aspiration window initially to minima and maxima. Also create counters for how many times the search has failed low and high. */
    int alpha = -20000, beta = 20000, failed_low_counter = 0, failed_high_counter = 0;

    /* Iterate through the depths */
    for ( int i = 0; i < depths.size (); ++i )
    {
        /* Run the search */
        ab_result_t new_ab_result = alpha_beta_search ( pc, depths.at ( i ), best_only, std::move ( ttable ), end_flag, ( finish_first && i == 0 ? chess_clock::time_point::max () : end_point ), alpha, beta );

        /* Extract the ttable from new_ab_result */
        ttable = std::move ( new_ab_result.ttable );

        /* If the search is incomplete, break */
        if ( new_ab_result.incomplete ) break;

        /* If there were no possible moves, break */
        if ( new_ab_result.moves.empty () ) break;

        /* If the search failed high or low, increase alpha or beta */
        if ( new_ab_result.failed_low  ) { alpha -= 100 * std::pow ( 5, failed_low_counter++  ); --i; } else
        if ( new_ab_result.failed_high ) { beta  += 100 * std::pow ( 5, failed_high_counter++ ); --i; } else

        /* Else the search was successful */
        {
            /* Set the latest result */
            ab_result = std::move ( new_ab_result );

            /* If this is the last depth, or every move is a losing checkmate, or every move is a winning checkmate, break */
            if ( i + 1 == depths.size () || ab_result.moves.front ().second <= -10000 || ab_result.moves.back ().second >= 10000 ) break;

            /* Reset the failed low and high counters */
            failed_low_counter = failed_high_counter = 0;

            /* Set both upper and lower bounds initially to the best and worst moves +- 25 */
            alpha = ab_result.moves.back  ().second - 25;
            beta  = ab_result.moves.front ().second + 25;

            /* If this depth was odd, and the next is even, the value is likely to go down, so decrease alpha by 50 */
            if ( ab_result.depth % 2 == 1 && depths.at ( i + 1 ) % 2 == 0 ) alpha -= 50; else

            /* If this depth was even, and the next is odd, the value is likely to go up, so increase beta by 50 */
            if ( ab_result.depth % 2 == 0 && depths.at ( i + 1 ) % 2 == 1 ) beta  += 50;
        }

        /* Get the predicted duration of the next search and cancel it if it will take too long. 
         * Note that if the last search was successful and this is the last search, the loop would have already ended. 
         */
        const chess_clock::duration pred_duration = 
            std::chrono::duration_cast<chess_clock::duration> ( std::pow ( 3.0, depths.at ( i + 1 ) - new_ab_result.depth ) * new_ab_result.duration );

        /* Force end the search now if this exceeds the end point */
        if ( chess_clock::now () + pred_duration > end_point ) break;
    }

    /* Move ttable back into ab_result */
    ab_result.ttable = std::move ( ttable );
    
    /* Return the result deepest complete search */
    return ab_result;
}




/* ALPHA BETA INTERNAL */



/** @name  alpha_beta_search_internal
 * 
 * @brief  Apply an alpha-beta search to a given depth.
 *         Note that although is non-const, a call to this function which does not throw will leave the object unmodified.
 * @param  pc: The color who's move it is next.
 * @param  bk_depth: The backwards depth, or the number of moves left before quiescence search.
 * @param  best_only: If true, the search will be optimised as only the best move is returned.
 * @param  end_flag: An atomic boolean, which when set to true, will end the search.
 * @param  end_point: A time point at which the search will end. Never by default.
 * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
 * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
 * @param  fd_depth: The forwards depth, or the number of moves since the root node, 0 by default.
 * @param  null_depth: The null depth, or the number of nodes that null move search has been active for, 0 by default.
 * @param  q_depth: The quiescence depth, or the number of nodes that quiescence search has been active for, 0 by default.
 * @return alpha_beta_t
 */
int chess::chessboard::alpha_beta_search_internal ( const pcolor pc, int bk_depth, const bool best_only, const std::atomic_bool& end_flag, const chess_clock::time_point end_point, int alpha, int beta, int fd_depth, int null_depth, int q_depth )
{

    /* CONSTANTS */

    /* Set the minimum bk_depth at which the transposition table will be searched and written to.
     * Should not be too small, as this will increase ttable usage and reduce its effectiveness (CPU cycles and memory usage).
     */
    constexpr int TTABLE_MIN_BK_DEPTH = 2;

    /* Set the minimum fd_depth at which the value from the transposition table will be used.
     * Should NOT be 0, since this may cause a too early cutoff.
     * Should be more than or equal to DRAW_MAX_FD_DEPTH.
     * Should also not be too small, as this will increase the likelihood of innaccuracies of affecting the search result.
     */
    constexpr int TTABLE_USE_VALUE_MIN_FD_DEPTH = 4;

    /* The maximum fd_depth at which a draw state will be detected. 
     * Depths less than (not equal to) DRAW_MAX_FD_DEPTH will be marked as draw-tainted inside the transposition table.
     * This will cause their value to only br trusted on a state of equal depth.
     * DRAW_MAX_FD_DEPTH should NOT be more than 4.
     * This would cause the draw condition to depend on variable search history which will cause inconsistencies if saved.
     * A value of 4 is a good idea, since it will detect the cycle long enough before it happens, so it can choose a best alternative move, if there is one.
     */
    constexpr int DRAW_MAX_FD_DEPTH = 4;

    /* Set the maximum depth quiescence search can go to.
     * This is important as it stops rare infinite loops relating to check in quiescence search.
     */
    constexpr int QUIESCENCE_MAX_Q_DEPTH = 16;

    /* The mimumum fd_depth that a null move may be tried.
     * Should be more than or equal to DRAW_MAX_FD_DEPTH.
     * Should not be too low, as this will increase the likelihood of innaccuracies of affecting the search result.
     */
    constexpr int NULL_MOVE_MIN_FD_DEPTH = 4;

    /* The change in bk_depth for a null move, and the amount of bk_depth that should be left over after reducing bk_depth.
     * The leftover depth should NOT be 0 (as this will introduce serious errors).
     * The maximum leftover depth reduces the overhead of computing a null move search to too large a depth.
     */
    constexpr int NULL_MOVE_CHANGE_BK_DEPTH = 2, NULL_MOVE_MIN_LEFTOVER_BK_DEPTH = 1, NULL_MOVE_MAX_LEFTOVER_BK_DEPTH = 5;

    /* The number of pieces such that if any player has less than this, the game is considered endgame */
    constexpr int ENDGAME_PIECES = 8;

    /* The minimum bk_depth at which an end flag or point cutoff is noticed.
     * Reading from the clock and atomically checking the end_flag is costly, so avoid doing it in later nodes.
     */
    constexpr int END_CUTOFF_MIN_BK_DEPTH = 4;



    /* MEMORY ACCESS FUNCTORS */

    /** @name  access_move_sets
     * 
     * @brief  Returns a reference to the array of moves for this depth
     * @param  pt: The piece to get the move array for
     * @return A reference to that array
     */
    auto access_move_sets = [ & ] ( ptype pt ) -> auto& { return ab_working->move_sets.at ( fd_depth ).at ( cast_penum ( pt ) ); };

    /** @name  access_killer_move
     * 
     * @brief  Returns a killer move for this depth
     * @param  index: The index of the killer move (0 or 1)
     * @return The killer move
     */
    auto access_killer_move = [ & ] ( int index ) -> auto& { return ab_working->killer_moves.at ( fd_depth ).at ( index ); };
 


    /* MEMORY MANAGEMENT */

    /* Clear the move sets, if this isn't the root node of internal iterative deepening */
    for ( auto& moves : ab_working->move_sets.at ( fd_depth ) ) moves.clear ();



    /* INFORMATION GATHERING */

    /* General game info */

    /* Get the other player */
    const pcolor npc = other_color ( pc );

    /* Get the check info of pc */
    const check_info_t check_info = get_check_info ( pc );

    /* Throw if the opposing king is in check */
    if ( is_in_check ( npc ) ) throw chess_internal_error { "Opposing color is in check in alpha_beta_search_internal ()." };

    /* Get king position */
    const int king_pos = bb ( pc, ptype::king ).trailing_zeros ();

    /* Get the primary and secondary propagator sets */
    const bitboard pp = ~bb (), sp = ~bb ( pc );

    /* Get the opposing concentation. True for north of board, false for south. */
    const bool opposing_conc = ( bb ( npc ) & bitboard { 0xffffffff00000000 } ).popcount () >= ( bb ( npc ) & bitboard { 0x00000000ffffffff } ).popcount ();

    /* Get the 7th and 8th ranks */
    const bitboard rank_8 { pc == pcolor::white ? bitboard::masks::rank_8 : bitboard::masks::rank_1 };
    const bitboard rank_7 { pc == pcolor::white ? bitboard::masks::rank_7 : bitboard::masks::rank_2 };
    const bitboard rank_7_and_8 = rank_7 | rank_8;

    /* Alpha-beta info */

    /* Store the current best value. Assume checkmate until improved upon. */
    int best_value = -10000 - bk_depth;

    /* Store the best move */
    move_t best_move;

    /* Store the original alpha */
    const int orig_alpha = alpha;

    /* add to the number of nodes visited */
    if ( bk_depth ) ++ab_working->num_nodes; else { ab_working->sum_q_depth += fd_depth; ++ab_working->num_q_nodes; }



    /* BOOLEAN FLAGS */

    /* Get whether we are in the endgame. Any of:
     * The number of pieces must be less than ENDGAME_PIECES.
     * There must be pieces other than the king and pawns.
     */
    const bool endgame = bb ( pcolor::white ).popcount () < ENDGAME_PIECES || bb ( pcolor::black ).popcount () < ENDGAME_PIECES
        || ( bb ( pcolor::white, ptype::queen ) | bb ( pcolor::white, ptype::rook ) | bb ( pcolor::white, ptype::bishop ) | bb ( pcolor::white, ptype::knight ) ).popcount () <= 2
        || ( bb ( pcolor::black, ptype::queen ) | bb ( pcolor::black, ptype::rook ) | bb ( pcolor::black, ptype::bishop ) | bb ( pcolor::black, ptype::knight ) ).popcount () <= 2;

    /* Get whether a draw cycle should be detected. All of:
     * Must not be trying a null move.
     * Must not be quiescing.
     * Must have fd_depth <= DRAW_MAX_FD_DEPTH.
     */
    const bool check_for_draw_cycle = !null_depth && !q_depth && fd_depth <= DRAW_MAX_FD_DEPTH;

    /* Get whether the ttable should be used. All of:
     * Must not be trying a null move.
     * Must not be quiescing.
     * Must have bk_depth >= TTABLE_MAX_FD_DEPTH
     */
    bool use_ttable = !null_depth && !q_depth && bk_depth >= TTABLE_MIN_BK_DEPTH;

    /* Get whether delta pruning should be used. All of:
     * Must not be the endgame.
     */
    const bool use_delta_pruning = !endgame;

    /* Whether should try a null move. All of:
     * Must not already be trying a null move.
     * Must not be quiescing.
     * Must not be the endgame.
     * Must not be in check.
     * Must have an fd_depth of at least NULL_MOVE_MIN_FD_DEPTH.
     * Reducing fd_depth by NULL_MOVE_CHANGE_BK_DEPTH must cause a leftover depth within the specified range.
     * Must not have a fd_depth less than DRAW_MAX_FD_DEPTH.
     */
    const bool use_null_move = !null_depth && !q_depth && !endgame && !check_info.check_count && fd_depth >= NULL_MOVE_MIN_FD_DEPTH
        && fd_depth >= DRAW_MAX_FD_DEPTH
        && bk_depth >= NULL_MOVE_MIN_LEFTOVER_BK_DEPTH + NULL_MOVE_CHANGE_BK_DEPTH 
        && bk_depth <= NULL_MOVE_MAX_LEFTOVER_BK_DEPTH + NULL_MOVE_CHANGE_BK_DEPTH;

    /* Whether a best move was found from the ttable */
    bool ttable_best_move = false;



    /* CHECK FOR A CYCLE */

    /* Only if the forwards depth is less than or equal to DRAW_MAX_FD_DEPTH.
     * Also there have been more than 8 moves (>= 9 states) in the game.
     */
    if ( check_for_draw_cycle && is_draw_state () ) return 0;


    
    /* TRY A LOOKUP */

    /* Try the ttable */
    if ( use_ttable )
    {
        /* Try to find the state */
        auto search_it = ab_working->ttable.find ( game_state_history.back () );

        /* See if an entry has been found */
        if ( search_it != ab_working->ttable.end () )
        {
            /* Extract the best move */
            best_move = search_it->second.best_move;

            /* Set to have found a best move */
            ttable_best_move = best_move.pt != ptype::no_piece;

            /* Only use the value if all of the below is true:
             * Has fd_depth >= TTABLE_USE_VALUE_MIN_FD_DEPTH.
             * The bk_depth of the ttable entry is more than or equal to that of this node.
             * If the entry is draw tainted, then the bk_depth of the ttable entry must be exactly equal to that of this node.
             */
            if ( fd_depth >= TTABLE_USE_VALUE_MIN_FD_DEPTH && bk_depth <= search_it->second.bk_depth && !( search_it->second.draw_tainted && bk_depth != search_it->second.bk_depth ) ) 
            {
                /* If the bound is exact, return the bound.
                 * If it is a lower bound, modify alpha.
                 * If it is an upper bound, modify beta.
                 */
                if ( search_it->second.bound == ab_ttable_entry_t::bound_t::exact ) return search_it->second.value;
                if ( search_it->second.bound == ab_ttable_entry_t::bound_t::lower ) alpha = std::max ( alpha, search_it->second.value ); else
                if ( search_it->second.bound == ab_ttable_entry_t::bound_t::upper ) beta  = std::min ( beta,  search_it->second.value );

                /* Possibly return now on an alpha-beta cutoff */
                if ( alpha >= beta ) return alpha;

                /* If we are deeper than the value in the ttable, then don't store new values in it */
                if ( bk_depth < search_it->second.bk_depth ) use_ttable = false;
            }
        }
    }



    /* CHECK FOR LEAF */

    /* If at bk_depth zero, start or continue with quiescence */
    if ( bk_depth == 0 )
    {
        /* Set q_depth if not already */
        if ( !q_depth ) q_depth = 1;

        /* Get static evaluation */
        best_value = evaluate ( pc );

        /* If in check increase the bk_depth by 1 */
        if ( check_info.check_count ) bk_depth++; else 
        {
            /* Else return now if exceeding the max quiescence depth */
            if ( q_depth >= QUIESCENCE_MAX_Q_DEPTH ) return best_value;

            /* Find the quiescence delta */
            const int quiescence_delta = std::max 
            ( { 100,
                bb ( npc, ptype::queen  ).is_nonempty () * 1100,
                bb ( npc, ptype::rook   ).is_nonempty () *  600,
                bb ( npc, ptype::bishop ).is_nonempty () *  400,
                bb ( npc, ptype::knight ).is_nonempty () *  400
            } ) + ( bb ( pc, ptype::pawn ) & rank_7 ).popcount () * 550;

            /* Or return on delta pruning if allowed */
            if ( use_delta_pruning && best_value + quiescence_delta < alpha ) return best_value;
        }

        /* Tune alpha to the static evaluation */
        alpha = std::max ( alpha, best_value );

        /* Return if alpha is greater than beta */
        if ( alpha >= beta ) return best_value;
    }



    /* TRY NULL MOVE */

    /* If null move is possible, try it */
    if ( use_null_move )
    {
        /* Make a null move */
        make_move_internal ( move_t { pc } );

        /* Apply the null move */
        int score = -alpha_beta_search_internal ( npc, bk_depth - NULL_MOVE_CHANGE_BK_DEPTH, best_only, end_flag, end_point, -beta, -beta + 1, fd_depth + 1, 1, ( q_depth ? q_depth + 1 : 0 ) );

        /* Unmake a null move */
        unmake_move_internal ();

        /* If there is an alpha-beta cutoff, even though this player missed a turn, then return beta.
         * This is because this position must be very powerful, so the other player is going to want to avoid it.
         * Don't return score, since the null move will cause extremes of values otherwise.
         */
        if ( score >= beta ) return beta;
    }



    /* APPLY MOVE FUNCTOR */

    /** @name  apply_move
     * 
     * @brief  Applies a move and recursively calls alpha_beta_search_internal on each of them
     * @param  move: The move to make
     * @return boolean, true for an alpha-beta cutoff, false otherwise
     */
    auto apply_move = [ & ] ( const move_t& move ) -> bool
    {
        /* Apply the move */
        make_move_internal ( move );

        /* Recursively call to get the value for this move.
        * Switch around and negate alpha and beta, since it is the other player's turn.
        * Since the next move is done by the other player, negate the value.
        */
        const int new_value = -alpha_beta_search_internal ( npc, ( bk_depth ? bk_depth - 1 : 0 ), best_only, end_flag, end_point, -beta, -alpha, fd_depth + 1, ( null_depth ? null_depth + 1 : 0 ), ( q_depth ? q_depth + 1 : 0 ) );

        /* Unmake the move */
        unmake_move_internal ();

        /* Set the best value and hence best move */
        if ( new_value > best_value ) { best_value = new_value; best_move = move; }

        /* Add to the number of moves made */
        if ( !q_depth ) ++ab_working->sum_moves; else ++ab_working->sum_q_moves;

        /* If end flag is set or past the end point, return true */
        if ( bk_depth >= END_CUTOFF_MIN_BK_DEPTH && ( end_flag || chess_clock::now () > end_point ) ) return true;

        /* If at the root node, add to the root moves. */
        if ( fd_depth == 0 ) ab_working->root_moves.push_back ( std::make_pair ( move, new_value ) );

        /* If the new best value is greater than alpha then:
         *     If this is not the root node, reassign alpha to the best value, else
         *     if this is the root node and best_only is true, reassign alpha to best value - 1 (-1 since this will avoid duplicate best values). 
         * If alpha is now greater than beta, return true due to an alpha-beta cutoff.
         */
        if ( fd_depth ) alpha = std::max ( alpha, best_value ); else if ( best_only ) alpha = std::max ( alpha, best_value - 1 );
        if ( alpha >= beta )
        {
            /* If the most recent killer move is similar, update its capture type, otherwise update the killer moves */
            if ( access_killer_move ( 0 ).is_similar ( move ) ) 
            {
                /* Only update the capture type if it was not previously a non-capture.
                    * If a move was killer and a non-capture, it must have been a very good move, so it's best to remember the fact it was a non-capture.
                    */
                if ( access_killer_move ( 0 ).capture_pt != ptype::no_piece ) access_killer_move ( 0 ).capture_pt = move.capture_pt; 
            } else
            {
                /* Swap the killer moves */
                std::swap ( access_killer_move ( 0 ), access_killer_move ( 1 ) );

                /* If the most recent killer move is now similar, update the capture type, else replace it */
                if ( access_killer_move ( 0 ).is_similar ( move ) ) 
                {
                    /* Only update the capture type if it was not previously a non-capture.
                        * If a move was killer and a non-capture, it must have been a very good move, so it's best to remember the fact it was a non-capture.
                        */
                    if ( access_killer_move ( 0 ).capture_pt != ptype::no_piece ) access_killer_move ( 0 ).capture_pt = move.capture_pt; 
                } else access_killer_move ( 0 ) = move;
            }    

            /* If is flagged to do so, add to the transposition table as a lower bound */
            if ( use_ttable ) ab_working->ttable.insert_or_assign ( game_state_history.back (), ab_ttable_entry_t { best_value, bk_depth, ab_ttable_entry_t::bound_t::lower, bk_depth < DRAW_MAX_FD_DEPTH, best_move } );

            /* Return */
            return true;
        }

        /* Return false */
        return false;
    };



    /* APPLY MOVE SET */

    /** @name  apply_move_set
     * 
     * @brief  Applies a set of moves in sequence
     * @param  pt: The type of the piece currently in focus
     * @param  from: The pos from which the piece is moving from
     * @param  move_set: The possible moves for that piece (not necessarily a singleton)
     * @return boolean, true for an alpha-beta cutoff, false otherwise
     */
    auto apply_move_set = [ & ] ( ptype pt, int from, bitboard move_set ) -> bool
    {
        /* Iterate through the individual moves */
        while ( move_set )
        {
            /* Get the position and singleton bitboard of the next move and unset that bit.
             * Choose motion towards the opposing color.
             */
            const int to = ( opposing_conc ? 63 - move_set.leading_zeros () : move_set.trailing_zeros () );
            move_set.reset ( to );

            /* Find the capture type */
            const ptype capture_pt = find_type ( npc, to );

            /* Detect if there are any pawns to promote */
            if ( pt == ptype::pawn && rank_8.test ( to ) )
            {
                /* Try the move with a queen and a knight as the promotion type, returning on alpha-beta cutoff */
                if ( apply_move ( move_t { pc, pt, capture_pt, ptype::queen,  from, to } ) ) return true;
                if ( apply_move ( move_t { pc, pt, capture_pt, ptype::knight, from, to } ) ) return true;
            } else

            /* Detect if this is an en passant capture */
            if ( pt == ptype::pawn && pc == aux_info.en_passant_color && to == aux_info.en_passant_target )
            {
                /* Apply the move and return on alpha-beta cutoff */
                if ( apply_move ( move_t { pc, pt, ptype::pawn, ptype::no_piece, from, to, true } ) ) return true;
            } else
            
            /* Else this is an ordinary move, so try it and return on alpha-beta cutoff */
            if ( apply_move ( move_t { pc, pt, capture_pt, ptype::no_piece, from, to } ) ) return true;
        }

        /* Return false */
        return false;
    };



    /* TRY BEST MOVE */

    /* Test if a best move has been found and try it if so */
    if ( ttable_best_move ) if ( apply_move ( best_move ) ) return best_value;



    /* COLLATE MOVE SETS */
    {
        /* Store whether there are any possible moves */
        bool pc_can_move = false;

        /* Iterate through the pieces */
        for ( const ptype pt : ptype_inc_value ) 
        {
            /* Iterate through pieces */
            for ( bitboard pieces = bb ( pc, pt ); pieces; )
            {
                /* Get the position of the next piece and reset that bit.
                * Favour the further away pieces to encourage them to move towards the other color.
                */
                const int pos = ( opposing_conc ? pieces.trailing_zeros () : 63 - pieces.leading_zeros () );
                pieces.reset ( pos );

                /* Get the move set */
                const bitboard move_set = get_move_set ( pc, pt, pos, check_info );

                /* Update pc_can_move */
                pc_can_move |= move_set.is_nonempty ();

                /* If the move set is non-empty or pt is the king, store the moves.
                 * The king's move set must be present to search for castling moves.
                 */
                if ( move_set || pt == ptype::king ) access_move_sets ( pt ).push_back ( std::make_pair ( pos, move_set ) );
            }
        }
    
        /* If there are no possible moves, return on a checkmate or stalemeate depending if in check or not */
        if ( !pc_can_move ) if ( check_info.check_count ) return -10000 - bk_depth; else return 0;
    }



    /* REMOVE BEST MOVE */

    /* If a best move was found, then it has already failed, so remove it from the move set */
    if ( ttable_best_move ) for ( auto& move_set : access_move_sets ( best_move.pt ) ) 
        if ( move_set.first == best_move.from && move_set.second.test ( best_move.to ) ) move_set.second.reset ( best_move.to );



    /* SEARCH */

    /* Look for killer moves */
    for ( const auto& killer_move : ab_working->killer_moves.at ( fd_depth ) )
    {
        /* Look for the move */
        if ( killer_move.pt != ptype::no_piece ) for ( auto& move_set : access_move_sets ( killer_move.pt ) )
        {
            /* See if this is the correct piece for the move */
            if ( move_set.first == killer_move.from && move_set.second.test ( killer_move.to ) )
            {
                /* Check that the move is the same capture, or is now a capture when previously it wasn't */
                if ( killer_move.capture_pt == ptype::no_piece || bb ( npc, killer_move.capture_pt ).test ( killer_move.to ) )
                {
                    /* Apply the killer move and return on alpha-beta cutoff */
                    if ( apply_move_set ( killer_move.pt, move_set.first, singleton_bitboard ( killer_move.to ) ) ) return best_value;
                    
                    /* Unset that bit in the move set */
                    move_set.second.reset ( killer_move.to );
                }

                /* killer move found, so break */
                break;
            }
        }
    }

    /* Look for pawn moves that promote that pawn */
    for ( auto& move_set : access_move_sets ( ptype::pawn ) )
    {
        /* Apply the move, then remove those bits */
        if ( apply_move_set ( ptype::pawn, move_set.first, move_set.second & rank_8 ) ) return best_value; 
        move_set.second &= ~rank_8;
    }

    /* Loop through the most valuable pieces to capture.
     * Look at the captures on pieces not protected by pawns first.
     */
    for ( const ptype captee_pt : ptype_dec_value ) 
    {
        /* Get this enemy type of piece */
        const bitboard enemy_captees = bb ( npc, captee_pt );

        /* If there are any of these enemy pieces to capture, look for a friendly piece that can capture them */
        if ( enemy_captees ) for ( const ptype captor_pt : ptype_inc_value )
        {
            /* Look though the different captors of this type */
            for ( const auto& move_set : access_move_sets ( captor_pt ) )
            {
                /* Try capturing, and return on alpha-beta cutoff */
                if ( apply_move_set ( captor_pt, move_set.first, move_set.second & enemy_captees ) ) return best_value;
            }
        }
    }

    /* Look for kingside castling moves */
    if ( access_move_sets ( ptype::king ).front ().second.test ( king_pos + 2 ) )
    {
        /* Try the move and return on alpha-beta cutoff */
        if ( apply_move_set ( ptype::king, king_pos, singleton_bitboard ( king_pos + 2 ) ) ) return best_value;

        /* Unset the move */
        access_move_sets ( ptype::king ).front ().second.reset ( king_pos + 2 );
    }

    /* Look for queenside castling moves */
    if ( access_move_sets ( ptype::king ).front ().second.test ( king_pos - 2 ) )
    {
        /* Try the move and return on alpha-beta cutoff */
        if ( apply_move_set ( ptype::king, king_pos, singleton_bitboard ( king_pos - 2 ) ) ) return best_value;

        /* Unset the move */
        access_move_sets ( ptype::king ).front ().second.reset ( king_pos - 2 );
    }

    /* Look for non-captures, unless quiescing */
    if ( bk_depth != 0 )
    {
        /* Iterate through all pieces */
        for ( const ptype pt : ptype_dec_move_value )
        {
            /* Loop though the pieces of this type */
            for ( const auto& move_set : access_move_sets ( pt ) )
            {
                /* Try the move, and return on alpha-beta cutoff */ 
                if ( apply_move_set ( pt, move_set.first, move_set.second & pp ) ) return best_value;
            }
        }
    }



    /* FINALLY */

    /* If is flagged to do so, add to the transposition table */
    if ( use_ttable ) ab_working->ttable.insert_or_assign ( game_state_history.back (), ab_ttable_entry_t
        { best_value, bk_depth, ( best_value <= orig_alpha ? ab_ttable_entry_t::bound_t::upper : ab_ttable_entry_t::bound_t::exact ), bk_depth < DRAW_MAX_FD_DEPTH, best_move } );

    /* Return the best value */
    return best_value;
}