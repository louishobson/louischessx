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



/* MAKING MOVES */



/** @name  make_move
 * 
 * @brief  Check a move is valid then apply it
 * @param  move: The move to apply
 * @return void
 */
void chess::chessboard::make_move ( const move_t& move )
{
    /* Check that the piece the move refers to exists */
    if ( !bb ( move.pc, move.pt ).test ( move.from ) ) throw std::runtime_error { "Invalid move initial position or type in make_move ()." };

    /* Check that the move is legal */
    if ( !get_move_set ( move.pc, move.pt, move.from, get_check_info ( move.pc ) ).test ( move.to ) ) throw std::runtime_error { "Illegal move final position in make_move ()." };

    /* If this is an en passant capture, ensure the captee is the most recent pawn to double push */
    if ( move.en_passant_pos )
    {
        if ( move.capture_pt != ptype::pawn || move.en_passant_pos != aux_info.double_push_pos || !bb ( other_color ( move.pc ), ptype::pawn ).test ( move.en_passant_pos ) ) throw std::runtime_error { "Invalid en passant position in make_move ()." };
    } else

    /* Else if this is a capture, ensure that the capture piece exists */
    if ( move.capture_pt != ptype::no_piece && !bb ( other_color ( move.pc ), move.capture_pt ).test ( move.to ) ) throw std::runtime_error { "Invalid capture type in make_move ()." }; else

    /* If this is a non-capture, ensure that there is no enemy piece in the final position */
    if ( move.capture_pt == ptype::no_piece &&  bb ( other_color ( move.pc ) ).test ( move.to ) ) throw std::runtime_error { "Invalid capture type in make_move ()." };

    /* Test if the move should be a pawn promotion and hence check the promote_pt is valid */
    if ( move.pt == ptype::pawn && ( move.pc == pcolor::white ? move.to >= 56 : move.to < 8 ) )
    {
        /* Move should promote, so check the promotion type is valid */
        if ( move.promote_pt != ptype::knight && move.promote_pt != ptype::bishop && move.promote_pt != ptype::rook && move.promote_pt != ptype::queen ) 
            throw std::runtime_error { "Invalid promotion type (move should promote) in make_move ()." };
    } else
    {
        /* Move should not promote, so check the promotion type is no_piece */
        if ( move.promote_pt != ptype::no_piece ) throw std::runtime_error { "Invalid promotion type (move should not promote) in make_move ()." };
    }

    /* Make the move */
    aux_info_t aux = make_move_internal ( move );

    /* Make sure the check count was what was expected */
    if ( move.check_count != get_check_info ( other_color ( move.pc ) ).check_count )
    {
        /* Unmake the move then throw */
        unmake_move_internal ( move, aux );
        throw std::runtime_error { "Invalid check count in make_move ()." };
    } 
}



/* SEARCH */



/* ALPHA-BETA SEARCH */

/** @name  alpha_beta_search
 * 
 * @brief  Set up and apply the alpha-beta search asynchronously.
 *         The board state is saved before return, so may be safely modified after returning but before resolution of the future.
 * @param  pc: The color whose move it is next.
 * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
 * @param  end_flag: An atomic boolean, which when set to true, will end the search. Can be unspecified.
 * @return A future to an ab_result_t struct.
 */
std::future<chess::chessboard::ab_result_t> chess::chessboard::alpha_beta_search ( const pcolor pc, const int depth, const std::atomic_bool& end_flag ) const
{
    /* Run this function asynchronously.
     * Be careful with lambda captures so that no references to this object are captured.
     */
    return std::async ( std::launch::async, [ =, cb = chessboard { * this }, &end_flag ] () mutable
    {
        /* Allocate new memory */
        cb.ab_working = new ab_working_t;

        /* Allocate excess memory  */
        cb.ab_working->move_sets.resize ( 128 );
        cb.ab_working->killer_moves.resize ( 128 );

        /* Reserve excess memory for root moves */
        cb.ab_working->root_moves.reserve ( 128 );

        /* Call and time the internal method */
        const auto t0 = std::chrono::steady_clock::now ();
        cb.alpha_beta_search_internal ( pc, depth, end_flag );
        const auto t1 = std::chrono::steady_clock::now ();

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

        /* Shrink to fit */
        ab_result.moves.shrink_to_fit ();

        /* Order the moves */
        std::sort ( ab_result.moves.begin (), ab_result.moves.end (), [] ( const auto& lhs, const auto& rhs ) { return lhs.second > rhs.second; } );

        /* Set the check count for each move */
        std::for_each ( ab_result.moves.begin (), ab_result.moves.end (), [ & ] ( auto& move ) 
        { 
            /* Make the move */
            const aux_info_t aux = cb.make_move_internal ( move.first ); 
            
            /* Get the check cound */
            move.first.check_count = cb.get_check_info ( other_color ( pc ) ).check_count;

            /* Unmake the move */
            cb.unmake_move_internal ( move.first, aux );
        } );

        /* Return the alpha beta result */
        return ab_result;
    } );
}

/** @name  alpha_beta_iterative_deepening
 * 
 * @brief  Apply an alpha-beta search over a range of depths asynchronously.
 *         The board state is saved before return, so may be safely modified after returning but before resolution of the future.
 *         Specifying an end point with a depth range of at least 3 will could to an early return.
 *         The method will predict the time for the 3rd and beyond depth using an exponential model, and return if will exceed the end point.
 * @param  pc: The color whose move it is next.
 * @param  min_depth: The lower bound of the depths to try.
 * @param  max_depth: The upper bound of the depths to try.
 * @param  threads: The number of threads to run simultaneously.
 * @param  end_flag: An atomic boolean, which when set to true, will end the search
 * @param  end_point: A time point at which the search will be automatically stopped. Never by default.
 * @param  finish_first: If true, always wait for the lowest depth search to finish, regardless of end_point or end_flag. True by default.
 * @return A future to an ab_result_t struct.
 */
std::future<chess::chessboard::ab_result_t> chess::chessboard::alpha_beta_iterative_deepening ( const pcolor pc, const int min_depth, const int max_depth, int threads, std::atomic_bool& end_flag, const std::chrono::steady_clock::time_point end_point, const bool finish_first ) const
{
    /* Run this function asynchronously.
     * Be careful with lambda captures so that no references to this object are captured.
     */
    return std::async ( std::launch::async, [ =, * this, &end_flag ] () mutable
    {
        /* If threads == 0, increase it to 1 */
        if ( threads == 0 ) threads = 1;

        /* Create an array of futures for each depth, and the times that they were started */
        std::vector<std::future<ab_result_t>> fts { max_depth - min_depth + 1u };
        std::vector<std::chrono::steady_clock::time_point> tps { max_depth - min_depth + 1u };

        /* Set of the first set of futures */
        for ( int i = 0; i < threads && min_depth + i <= max_depth; ++i ) 
        {
            /* Don't pass the lowest depth search end_flag if finish_first is set */
            if ( i == 0 && finish_first ) fts.at ( i ) = alpha_beta_search ( pc, min_depth + i, false ); else fts.at ( i ) = alpha_beta_search ( pc, min_depth + i, end_flag );

            /* Set the start point */
            tps.at ( i ) = std::chrono::steady_clock::now ();
        }

        /* The result of the highest depth complete search */
        ab_result_t ab_result;

        /* Wait for each future in turn */
        for ( int i = 0; min_depth + i <= max_depth; ++i )
        {
            /* If the future is invalid, break */
            if ( !fts.at ( i ).valid () ) break;

            /* Wait until the time point for the next result */
            if ( i == 0 && finish_first ) fts.at ( i ).wait (); else fts.at ( i ).wait_until ( end_point );

            /* Only accept the new moves if not past the end point and the end flag is unset */
            if ( std::chrono::steady_clock::now () < end_point && !end_flag ) 
            {
                /* Get the new alpha beta result */
                ab_result_t new_ab_result = fts.at ( i ).get ();

                /* Start the next search if there are any left to start */
                if ( min_depth + i + threads <= max_depth ) 
                {
                    fts.at ( i + threads ) = alpha_beta_search ( pc, min_depth + i + threads, end_flag );
                    tps.at ( i + threads ) = std::chrono::steady_clock::now ();
                }

                /* If this is not the last search, predict when the next thread will finish */
                if ( min_depth + i != max_depth )
                {
                    /* Get the predicted duration */
                    const std::chrono::steady_clock::duration pred_duration = std::chrono::duration_cast<std::chrono::steady_clock::duration> ( ( new_ab_result.duration / std::chrono::duration<double, std::nano> { ab_result.duration } ) * 0.9 * new_ab_result.duration );

                    /* Force end the search now if this exceeds the end point */
                    if ( tps.at ( i + 1 ) + pred_duration > end_point ) end_flag = true;
                }

                /* Set the latest result */
                ab_result = new_ab_result;
            } else

            /* Else if the search has exceeded the end point or has otherwise been cancelled */ 
            {
                /* If this is the lowest depth search and finish_first is set, still accept the result */
                if ( i == 0 && finish_first ) ab_result = fts.at ( i ).get ();

                /* Set the end flag to true */
                end_flag = true;
            }
        }
        
        /* Return the result deepest complete search */
        return ab_result;
    } );
}

/** @name  alpha_beta_search_internal
 * 
 * @brief  Apply an alpha-beta search to a given depth.
 *         Note that although is non-const, a call to this function which does not throw will leave the object unmodified.
 * @param  pc: The color who's move it is next
 * @param  bk_depth: The backwards depth, or the number of moves left before quiescence search
 * @param  end_flag: An atomic boolean, which when set to true, will end the search.
 * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
 * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
 * @param  fd_depth: The forwards depth, or the number of moves since the root node, 0 by default.
 * @param  null_depth: The null depth, or the number of nodes that null move search has been active for, 0 by default.
 * @param  q_depth: The quiescence depth, or the number of nodes that quiescence search has been active for, 0 by default.
 * @return alpha_beta_t
 */
int chess::chessboard::alpha_beta_search_internal ( const pcolor pc, int bk_depth, const std::atomic_bool& end_flag, int alpha, int beta, int fd_depth, int null_depth, int q_depth )
{

    /* CONSTANTS */

    /* Set the fd_depth range in which the transposition table will be used.
     * Increased memory usage due to exponential growth of the search tree make using the ttable at higher fd_depths undesirable.
     */
    constexpr int TTABLE_MIN_SEARCH_FD_DEPTH = 3, TTABLE_MAX_SEARCH_FD_DEPTH = 6;
    constexpr int TTABLE_MIN_STORE_FD_DEPTH  = 3, TTABLE_MAX_STORE_FD_DEPTH  = 6;

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

    /* Get the original alpha */
    const int orig_alpha = alpha;

    /* Get the current alpha-beta state */
    const ab_state_t ab_state { * this, pc };

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
    bool use_ttable = !null_depth && !q_depth && fd_depth <= TTABLE_MAX_SEARCH_FD_DEPTH && fd_depth >= TTABLE_MIN_SEARCH_FD_DEPTH;

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


    
    /* TRY A LOOKUP */

    /* Try the ttable */
    if ( use_ttable )
    {
        /* Try to find the state */
        auto search_it = ab_working->ttable.find ( ab_state );

        /* If an entry is found and we are at least as deep as the entry, either return its value or modify alpha and beta */
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

    /* If we are outside the specified fd_depth range, set use_ttable to false to stop storing new values in the ttable */
    if ( fd_depth > TTABLE_MAX_STORE_FD_DEPTH || fd_depth < TTABLE_MIN_STORE_FD_DEPTH ) use_ttable = false;



    /* CHECK FOR LEAF */

    /* If at bk_depth zero, start or continue with quiescence */
    int static_eval = 0;
    if ( bk_depth == 0 )
    {
        /* Set q_depth if not already */
        if ( !q_depth ) q_depth = 1;

        /* Get static evaluation */
        static_eval = evaluate ( pc );

        /* If in check increase the bk_depth by 1 */
        if ( check_info.check_count ) bk_depth++; else 
        {
            /* Else return now if exceeding the max quiescence depth */
            if ( q_depth >= QUIESCENCE_MAX_Q_DEPTH ) return static_eval;

            /* Find the quiescence delta */
            const int quiescence_delta = std::max 
            ( { 100,
                bb ( npc, ptype::queen  ).is_nonempty () * 1100,
                bb ( npc, ptype::rook   ).is_nonempty () *  600,
                bb ( npc, ptype::bishop ).is_nonempty () *  400,
                bb ( npc, ptype::knight ).is_nonempty () *  400
            } ) + ( bb ( pc, ptype::pawn ) & rank_7 ).popcount () * 550;

            /* Or return on delta pruning if allowed */
            if ( use_delta_pruning && static_eval + quiescence_delta < alpha ) return static_eval;
        }

        /* Return if static evaluation is greater than beta */
        if ( static_eval >= beta ) return beta;

        /* Tune alpha to the static evaluation */
        alpha = std::max ( alpha, static_eval );
    }



    /* TRY NULL MOVE */

    /* If null move is possible, try it */
    if ( use_null_move )
    {
        /* Make a null move */
        const aux_info_t aux = make_move_internal ( move_t {} );

        /* Apply the null move */
        int score = -alpha_beta_search_internal ( npc, bk_depth - NULL_MOVE_CHANGE_BK_DEPTH, end_flag, -beta, -beta + 1, fd_depth + 1, 1, ( q_depth ? q_depth + 1 : 0 ) );

        /* Unmake a null move */
        unmake_move_internal ( move_t {}, aux );

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
        const aux_info_t aux = make_move_internal ( move );

        /* Recursively call to get the value for this move.
        * Switch around and negate alpha and beta, since it is the other player's turn.
        * Since the next move is done by the other player, negate the value.
        */
        const int new_value = -alpha_beta_search_internal ( npc, ( bk_depth ? bk_depth - 1 : 0 ), end_flag, -beta, -alpha, fd_depth + 1, ( null_depth ? null_depth + 1 : 0 ), ( q_depth ? q_depth + 1 : 0 ) );

        /* Unmake the move */
        unmake_move_internal ( move, aux ); 

        /* Add to the number of moves made */
        if ( !q_depth ) ++ab_working->sum_moves; else ++ab_working->sum_q_moves;

        /* If past the end point, return */
        if ( fd_depth <= END_FLAG_CUTOFF_MAX_FD_DEPTH && end_flag ) return true;

        /* If at the root node, add to the root moves. */
        if ( fd_depth == 0 ) ab_working->root_moves.push_back ( std::make_pair ( move, new_value ) ); else

        /* Otherwise consider alpha-beta pruning etc. */
        {
            /* If the new value is greater than the best value, then reassign the best value.
            * Further check if the new best value is greater than alpha, if so reassign alpha.
            * If alpha is now greater than beta, return true due to an alpha-beta cutoff.
            */
            best_value = std::max ( best_value, new_value );
            alpha = std::max ( alpha, best_value );
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
                if ( use_ttable ) ab_working->ttable.insert_or_assign ( ab_state, ab_ttable_entry_t { best_value, bk_depth, ab_ttable_entry_t::bound_t::lower } );

                /* Return */
                return true;
            }
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
            /* Get the position and singleton bitboard of the next move.
             * Choose motion towards the opposing color.
             */
            const int to = ( opposing_conc ? 63 - move_set.leading_zeros () : move_set.trailing_zeros () );

            /* Unset this bit in the set of moves */
            move_set.reset ( to );

            /* Detect if there are any pawns to promote */
            if ( pt == ptype::pawn && rank_8.test ( to ) )
            {
                /* Try the move with a queen and a knight as the promotion type, returning on alpha-beta cutoff */
                if ( apply_move ( move_t { pc, pt, find_type ( npc, to ), ptype::queen,  from, to, 0, 0 } ) ) return true;
                if ( apply_move ( move_t { pc, pt, find_type ( npc, to ), ptype::knight, from, to, 0, 0 } ) ) return true;
            } else

            /* Detect if this is an en passant capture. 
             * "( to - from ) % 8" is non-zero if the pawn move is a capture. 
             * bb ( npc ).test ( to ) is false if the move does not land on an enemy piece.
             */
            if ( pt == ptype::pawn && ( to - from ) % 8 != 0 && !bb ( npc ).test ( to ) )
            {
                /* Apply the move and return on alpha-beta cutoff */
                if ( apply_move ( move_t { pc, pt, ptype::pawn, ptype::no_piece, from, to, ( from / 8 ) * 8 + ( to % 8 ), 0 } ) ) return true;
            } else
            
            /* Else this is an ordinary move, so try it and return on alpha-beta cutoff */
            if ( apply_move ( move_t { pc, pt, find_type ( npc, to ), ptype::no_piece, from, to, 0, 0 } ) ) return true;
        }

        /* Return false */
        return false;
    };



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
    if ( use_ttable ) ab_working->ttable.insert_or_assign ( ab_state, ab_ttable_entry_t
    {
        best_value, bk_depth, ( best_value <= orig_alpha ? ab_ttable_entry_t::bound_t::upper : ab_ttable_entry_t::bound_t::exact )
    } );

    /* Return the best value */
    return best_value;
}