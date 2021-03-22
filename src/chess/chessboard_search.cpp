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



/* ALPHA BETA SEARCH */



/** @name  alpha_beta_search
 * 
 * @brief  Set up and apply the alpha-beta search asynchronously.
 *         The board state is saved before return, so may be safely modified after returning but before resolution of the future.
 * @param  pc: The color whose move it is next.
 * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
 * @param  best_only: If true, the search will be optimised as only the best move is returned.
 * @param  end_flag: An atomic boolean, which when set to true, will end the search. Can be unspecified.
 * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
 * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
 * @return A future to an ab_result_t struct.
 */
std::future<chess::chessboard::ab_result_t> chess::chessboard::alpha_beta_search ( const pcolor pc, const int depth, const bool best_only, const std::atomic_bool& end_flag, const int alpha, const int beta ) const
{
    /* Run this function asynchronously.
     * Be careful with lambda captures so that no references to this object are captured.
     */
    return std::async ( std::launch::async, [ =, cb { copy_and_move_ab_working () }, &end_flag ] () mutable
    {
        /* Allocate new memory if necessary */
        if ( !cb.ab_working ) cb.ab_working.reset ( new ab_working_t );

        /* Allocate excess memory  */
        cb.ab_working->move_sets.resize ( 32 );
        cb.ab_working->killer_moves.resize ( 32 );

        /* Reserve excess memory for root moves */
        cb.ab_working->root_moves.reserve ( 32 );

        /* Call and time the internal method */
        const auto t0 = chess_clock::now ();
        cb.alpha_beta_search_internal ( pc, depth, best_only, end_flag, alpha, beta );
        const auto t1 = chess_clock::now ();

        /* Create the ab result struct */
        ab_result_t ab_result;

        /* Set the simple information */
        ab_result.depth       = depth; 
        ab_result.num_nodes   = cb.ab_working->num_nodes;
        ab_result.num_q_nodes = cb.ab_working->num_q_nodes;
        ab_result.av_q_depth  = cb.ab_working->sum_q_depth / static_cast<double> ( cb.ab_working->num_q_nodes );
        ab_result.av_moves    = cb.ab_working->sum_moves   / static_cast<double> ( cb.ab_working->num_nodes   );
        ab_result.av_q_moves  = cb.ab_working->sum_q_moves / static_cast<double> ( cb.ab_working->num_q_nodes );
        ab_result.duration    = t1 - t0;

        /* Extract the move list */
        ab_result.moves = std::move ( cb.ab_working->root_moves );

        /* Extract the alpha beta working values */
        ab_result._ab_working = std::move ( cb.ab_working );

        /* If there are no possible moves, return now */
        if ( ab_result.moves.size () == 0 ) return ab_result;

        /* Order the moves */
        std::sort ( ab_result.moves.begin (), ab_result.moves.end (), [] ( const auto& lhs, const auto& rhs ) { return lhs.second > rhs.second; } );

        /* Resize the moves list to 1 if best_only is set */
        if ( best_only ) ab_result.moves.resize ( 1 );

        /* Set the check count and checkmate info for each move */
        std::for_each ( ab_result.moves.begin (), ab_result.moves.end (), [ & ] ( auto& move ) 
        { 
            /* Make the move */
            cb.make_move_internal ( move.first ); 
            
            /* Get the check bool */
            move.first.check = cb.get_check_info ( other_color ( pc ) ).check_count;

            /* Get if is a checkmate */
            move.first.checkmate = ( move.second == 10000 + depth - 1 );

            /* Unmake the move */
            cb.unmake_move_internal ();
        } );

        /* Return the alpha beta result */
        return ab_result;
    } );
}



/* ITERATIVE DEEPENING */



/** @name  alpha_beta_iterative_deepening
 * 
 * @brief  Apply an alpha-beta search over a range of depths asynchronously.
 *         The board state is saved before return, so may be safely modified after returning but before resolution of the future.
 *         Specifying an end point could to an early return rather than starting later searches.
 * @param  pc: The color whose move it is next.
 * @param  depths: A list of depth values to search.
 * @param  best_only: If true, the search will be optimised as only the best move is returned.
 * @param  end_flag: An atomic boolean, which when set to true, will end the search.
 * @param  end_point: A time point at which the search will be automatically stopped. Never by default.
 * @param  finish_first: If true, always wait for the lowest depth search to finish, regardless of end_point or end_flag. True by default.
 * @return A future to an ab_result_t struct.
 */
std::future<chess::chessboard::ab_result_t> chess::chessboard::alpha_beta_iterative_deepening ( const pcolor pc, const std::vector<int>& depths, const bool best_only, std::atomic_bool& end_flag, const chess_clock::time_point end_point, const bool finish_first ) const
{
    /* Run this function asynchronously.
     * Be careful with lambda captures so that no references to this object are captured.
     */
    return std::async ( std::launch::async, [ =, cb { copy_and_move_ab_working () }, &end_flag ] () mutable
    {
        /* The result of the highest depth complete search */
        ab_result_t ab_result;

        /* Exponents for high and low windows for if previous iteration failed high or low */
        int low_exponent = 1, high_exponent = 1;

        /* Iterate through the depths */
        for ( int i = 0; i < depths.size (); ++i )
        {
            /* Store the result */
            ab_result_t new_ab_result;

            /* Get the aspiration window sized based on whether the previous and this depth are both odd/even or are different */
            int low_window = 50, high_window = 50;
            if ( ab_result.depth % 2 == 1 && depths.at ( i ) % 2 == 0 ) low_window  = 100;
            if ( ab_result.depth % 2 == 0 && depths.at ( i ) % 2 == 1 ) high_window = 100;

            /* Get the aspiration window to use */
            const int alpha = ( ab_result.moves.size () ? ab_result.moves.front ().second - std::pow ( low_window,  low_exponent  ) : -20000 );
            const int beta  = ( ab_result.moves.size () ? ab_result.moves.front ().second + std::pow ( high_window, high_exponent ) : +20000 );

            /* Start the search */
            auto new_ab_result_future = cb.alpha_beta_search ( pc, depths.at ( i ), best_only, end_flag, alpha, beta );

            /* Wait for the search to finish or time out. If it times out or is force ended, ensure the end flag is set, wait for the search to finish and break. */
            if ( new_ab_result_future.wait_until ( i == 0 && finish_first ? chess_clock::time_point::max () : end_point ) != std::future_status::ready || end_flag ) { end_flag = true; new_ab_result_future.wait (); break; }

            /* Get the new result */
            new_ab_result = new_ab_result_future.get ();

            /* Move the ab_working values from the search into cb */
            cb.ab_working = std::move ( new_ab_result._ab_working );

            /* Detect an incorrect aspiration window, and whether the search failed high or low. If so, increase the high/low_exponent counters. */
            if ( new_ab_result.moves.size () && new_ab_result.moves.front ().second <= alpha ) { while ( ab_result.moves.front ().second - std::pow ( low_window,  ++low_exponent  ) > new_ab_result.moves.front ().second ); --i; } else
            if ( new_ab_result.moves.size () && new_ab_result.moves.front ().second >= beta  ) { while ( ab_result.moves.front ().second + std::pow ( high_window, ++high_exponent ) < new_ab_result.moves.front ().second ); --i; } else

            /* Else the search was successful */
            {
                /* Reset aspiration window exponents */
                low_exponent = high_exponent = 1;

                /* Set the latest result */
                ab_result = std::move ( new_ab_result );

                /* If the latest result is a checkmate for either color, break immediately */
                if ( ab_result.moves.empty () || ab_result.moves.front ().second >= 10000 ) break;
            }

            /* If this is not the last search, get the predicted duration of the next search and cancel it if it will take too long */
            if ( i != depths.size () - 1 )
            {
                /* Get the predicted duration */
                const chess_clock::duration pred_duration = 
                    std::chrono::duration_cast<chess_clock::duration> ( std::pow ( new_ab_result.av_moves, depths.at ( i + 1 ) - new_ab_result.depth ) * new_ab_result.duration );

                /* Force end the search now if this exceeds the end point */
                if ( chess_clock::now () + pred_duration > end_point ) break;
            }
        }

        /* Copy out ab_working from cb */
        ab_result._ab_working = std::move ( cb.ab_working );
        
        /* Return the result deepest complete search */
        return ab_result;
    } );
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
 * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
 * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
 * @param  fd_depth: The forwards depth, or the number of moves since the root node, 0 by default.
 * @param  null_depth: The null depth, or the number of nodes that null move search has been active for, 0 by default.
 * @param  q_depth: The quiescence depth, or the number of nodes that quiescence search has been active for, 0 by default.
 * @return alpha_beta_t
 */
int chess::chessboard::alpha_beta_search_internal ( const pcolor pc, int bk_depth, const bool best_only, const std::atomic_bool& end_flag, int alpha, int beta, int fd_depth, int null_depth, int q_depth )
{

    /* CONSTANTS */

    /* Set the maximum fd_depth at which the transposition table will be searched and written to */
    constexpr int TTABLE_MAX_FD_DEPTH = 6;

    /* The maximum fd_depth at which a draw state will be detected. 
     * Depths before this will stop best_value from being stored in the tranposition table.
     */
    constexpr int DRAW_MAX_FD_DEPTH = 3;

    /* Set the maximum depth quiescence search can go to.
     * This is important as it stops rare infinite loops relating to check in quiescence search.
     */
    constexpr int QUIESCENCE_MAX_Q_DEPTH = 16;

    /* The mimumum fd_depth that a null move may be tried */
    constexpr int NULL_MOVE_MIN_FD_DEPTH = 3;

    /* The change in bk_depth for a null move, and the amount of bk_depth that should be left over after reducing bk_depth */
    constexpr int NULL_MOVE_CHANGE_BK_DEPTH = 2, NULL_MOVE_MIN_LEFTOVER_BK_DEPTH = 2, NULL_MOVE_MAX_LEFTOVER_BK_DEPTH = 5;

    /* The number of pieces such that if any player has less than this, the game is considered endgame */
    constexpr int ENDGAME_PIECES = 8;

    /* The maximum fd_depth at which an end flag cutoff is noticed */
    constexpr int END_FLAG_CUTOFF_MAX_FD_DEPTH = 5;



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
#if CHESS_VALIDATE
    if ( is_in_check ( npc ) ) throw std::runtime_error { "Opposing color is in check in alpha_beta_search_internal ()." };
#endif

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

    /* Store the current best value.
     * Losing sooner is worse (and winning sooner is better), which the minus depth accounts for
     */
    int best_value = -10000 - bk_depth;

    /* Store the best move */
    move_t best_move;

    /* Get the original alpha */
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

    /* Get whether the ttable should be used. All of:
     * Must not be trying a null move.
     * Must not be quiescing.
     * Must be within the specified fd_depth range.
     */
    bool use_ttable = !null_depth && !q_depth && fd_depth <= TTABLE_MAX_FD_DEPTH;

    /* Get whether delta pruning should be used. All of:
     * Must not be the endgame.
     */
    const bool use_delta_pruning = !endgame;

    /* Whether should try a null move. All of:
     * Must not already be trying a null move.
     * Must not be the endgame
     * Must not be quiescing.
     * Must have an fd_depth of at least NULL_MOVE_MIN_FD_DEPTH.
     * Reducing depth by NULL_MOVE_CHANGE_BK_DEPTH must cause a leftover depth within the specified range
     * Must not be in check.
     */
    const bool use_null_move = !null_depth && !endgame && !q_depth && fd_depth >= NULL_MOVE_MIN_FD_DEPTH && !check_info.check_vectors 
        && bk_depth >= NULL_MOVE_MIN_LEFTOVER_BK_DEPTH + NULL_MOVE_CHANGE_BK_DEPTH 
        && bk_depth <= NULL_MOVE_MAX_LEFTOVER_BK_DEPTH + NULL_MOVE_CHANGE_BK_DEPTH;

    /* Whether a best move was found from the ttable */
    bool ttable_best_move = false;



    /* CHECK FOR A CYCLE */

    /* Only if the forwards depth is less than or equal to DRAW_MAX_FD_DEPTH.
     * Also there have been more than 8 moves (>= 9 states) in the game.
     */
    if ( fd_depth <= DRAW_MAX_FD_DEPTH && game_state_history.size () >= 9 )
    {
        /* Check if the last 8 moves have moved us in two cycles */
        if ( game_state_history.back () == game_state_history.at ( game_state_history.size () - 5 ) && game_state_history.back () == game_state_history.at ( game_state_history.size () - 9 ) ) return 0;
    }


    
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

            /* If we are at least as deep as the entry, either return its value or modify alpha and beta */
            if ( search_it != ab_working->ttable.end () && bk_depth <= search_it->second.bk_depth )
            {
                /* If we are deeper than the value in the ttable, then don't store new values in it */
                if ( bk_depth < search_it->second.bk_depth ) use_ttable = false;

                /* If the bound is exact, return the entry's value.
                * If it is a lower bound, modify alpha.
                * If it is an upper bound, modify beta.
                */
                if ( search_it->second.bound == ab_ttable_entry_t::bound_t::exact ) return search_it->second.value;
                if ( search_it->second.bound == ab_ttable_entry_t::bound_t::lower ) alpha = std::max ( alpha, search_it->second.value ); else
                if ( search_it->second.bound == ab_ttable_entry_t::bound_t::upper ) beta  = std::min ( beta,  search_it->second.value );

                /* Possibly return now on an alpha-beta cutoff */
                if ( alpha >= beta ) return alpha; 
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
        int score = -alpha_beta_search_internal ( npc, bk_depth - NULL_MOVE_CHANGE_BK_DEPTH, best_only, end_flag, -beta, -beta + 1, fd_depth + 1, 1, ( q_depth ? q_depth + 1 : 0 ) );

        /* Unmake a null move */
        unmake_move_internal ();

        /* If proved successful, return beta.
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
        const int new_value = -alpha_beta_search_internal ( npc, ( bk_depth ? bk_depth - 1 : 0 ), best_only, end_flag, -beta, -alpha, fd_depth + 1, ( null_depth ? null_depth + 1 : 0 ), ( q_depth ? q_depth + 1 : 0 ) );

        /* Unmake the move */
        unmake_move_internal (); 

        /* Set the best value and hence best move */
        if ( new_value > best_value ) { best_value = new_value; best_move = move; }

        /* Add to the number of moves made */
        if ( !q_depth ) ++ab_working->sum_moves; else ++ab_working->sum_q_moves;

        /* If past the end point, return */
        if ( fd_depth <= END_FLAG_CUTOFF_MAX_FD_DEPTH && end_flag ) return true;

        /* If at the root node, add to the root moves. */
        if ( fd_depth == 0 ) ab_working->root_moves.push_back ( std::make_pair ( move, new_value ) );

        /* If the new value is better than the best value, update the best value and move.
         * If the new best value is greater than alpha then:
         *     If this is not the root node, reassign alpha to the best value, else
         *     if this is the root node and best_only is true, reassign alpha to best value - 1 (-1 since this will avoid duplicate best values). 
         * If alpha is now greater than beta, return true due to an alpha-beta cutoff.
         */
        if ( new_value > best_value ) { best_value = new_value; best_move = move; }
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
            if ( use_ttable ) if ( fd_depth >= DRAW_MAX_FD_DEPTH ) 
                ab_working->ttable.insert_or_assign ( game_state_history.back (), ab_ttable_entry_t { best_value, bk_depth, ab_ttable_entry_t::bound_t::lower, best_move } ); else
                ab_working->ttable.insert_or_assign ( game_state_history.back (), ab_ttable_entry_t { -10000    , bk_depth, ab_ttable_entry_t::bound_t::lower, best_move } );

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
            if ( aux_info.double_push_pos && pt == ptype::pawn && ( to - aux_info.double_push_pos ) == ( pc == pcolor::white ? +8 : -8 ) )
            {
                /* Apply the move and return on alpha-beta cutoff */
                if ( apply_move ( move_t { pc, pt, ptype::pawn, ptype::no_piece, from, to, aux_info.double_push_pos } ) ) return true;
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
        /* Iterate through the pieces */
        #pragma clang loop unroll ( full )
        #pragma GCC unroll 6
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

                /* If the move set is non-empty or pt is the king, store the moves.
                 * The king's move set must be present to search for castling moves.
                 */
                if ( move_set || pt == ptype::king ) access_move_sets ( pt ).push_back ( std::make_pair ( pos, move_set ) );
            }
        }
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
    if ( use_ttable ) if ( fd_depth >= DRAW_MAX_FD_DEPTH ) ab_working->ttable.insert_or_assign ( game_state_history.back (), ab_ttable_entry_t
        {
            best_value, bk_depth, ( best_value <= orig_alpha ? ab_ttable_entry_t::bound_t::upper : ab_ttable_entry_t::bound_t::exact ), best_move
        } ); else
        ab_working->ttable.insert_or_assign ( game_state_history.back (), ab_ttable_entry_t { -10000, bk_depth, ab_ttable_entry_t::bound_t::lower, best_move } );

    /* Return the best value */
    return best_value;
}