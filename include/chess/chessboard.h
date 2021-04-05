/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/chessboard.h
 * 
 * Header file for managing a chessboard
 * 
 */



/* HEADER GUARD */
#ifndef CHESSBOARD_H_INCLUDED
#define CHESSBOARD_H_INCLUDED



/* INCLUDES */
#include <algorithm>
#include <atomic>
#include <chess/bitboard.h>
#include <chess/macros.h>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <future>
#include <memory>
#include <unordered_map>
#include <regex>
#include <string>
#include <utility>
#include <vector>



/* DECLARATIONS */

namespace chess
{

    /* PIECE ENUMS */

    /* enum pcolor
     *
     * Enum values for different colors of piece (not types)
     */
    enum class pcolor
    {
        white,
        black,
        no_piece
    };

    /* enum ptype
     *
     * Enum values for different types of piece (not colors)
     */
    enum class ptype
    {
        pawn,
        knight,
        bishop,
        rook,
        queen,
        king,
        any_piece,
        no_piece
    };

    /* ptype_inc/dec_value
     *
     * Arrays of ptypes of increasing and decreasing value
     */
    inline constexpr std::array<ptype, 6> ptype_inc_value { ptype::pawn, ptype::knight, ptype::bishop, ptype::rook, ptype::queen, ptype::king };
    inline constexpr std::array<ptype, 6> ptype_dec_value { ptype::king, ptype::queen, ptype::rook, ptype::bishop, ptype::knight, ptype::pawn };

    /* ptype_dec_move_value
     *
     * Array of ptypes of decreasing value of moving
     */
    inline constexpr std::array<ptype, 6> ptype_dec_move_value { ptype::queen, ptype::rook, ptype::bishop, ptype::knight, ptype::pawn, ptype::king };

    /** @name  bool_color
     * 
     * @brief  cast a piece color to a bool
     * @param  pc: The piece color to cast. Undefined behavior if is no_piece.
     * @return bool
     */
    constexpr bool bool_color ( pcolor pc ) noexcept;

    /** @name  other_color
     * 
     * @brief  Take a piece color and give the other color
     * @param  pc: The piece color to switch. Undefined behavior if is no_piece.
     * @return The other color of piece
     */
    constexpr pcolor other_color ( pcolor pc ) noexcept;

    /** @name  cast_penum
     * 
     * @brief  Casts a penum to its underlying type
     * @param  pc: The piece color to cast
     * @param  pt: The piece type to cast
     * @return int
     */
    constexpr int cast_penum ( pcolor pc ) noexcept;
    constexpr int cast_penum ( ptype  pt ) noexcept;

    /** @name  check_penum
     * 
     * @brief  This function does nothing if validation is disabled.
     *         Otherwise, it takes a pc and pt to be used to access the bbs array, and validate that they are possible.
     * @param  pc: The piece color
     * @param  pt: The piece type. Defaults to any_piece (which will pass).
     * @return void, but will throw if validation is enabled and pc or pt are out of range.
     */
    static void check_penum ( pcolor pc, ptype pt = ptype::any_piece ) chess_validate_throw;
    static void check_penum ( ptype pt ) chess_validate_throw;



    /* CLOCK TYPEDEF */

    /* The clock to use with timing */
    typedef std::chrono::system_clock chess_clock;



    /* CHESS EXCEPTION STRUCTS */

    /* struct chess_input_error
     *
     * Thrown by chess methods when input is invalid. The board is always left in the same state as before the method was called, if this exception is thrown.
     */
    struct chess_input_error;

    /* struct chess_internal_error
     *
     * Thrown by chess methods when an internal error has occured. The board is left in an undefined state, if this exception is thrown.
     */
    struct chess_internal_error;



    /* MOVE CLASS */
    
    /* class move_t
     *
     * Stores a move in a chess game
     */
    class move_t;



    /* CHESSBOARD CLASS */

    /* class chessboard 
     *
     * Store and manipulate a bitboard-based chessboard
     */
    class chessboard;



    /* GAME CONTROLLER CLASS */

    /* class game_controller
     *
     * An instance will store and maintain a chess game based on a set of streams communicating over the UCI protocol
     */
    class game_controller;

}



/* CHESS EXCEPTION STRUCTS */

/* struct chess_input_error
 *
 * Thrown by chess methods when input is invalid. The board is always left in the same state as before the method was called, if this exception is thrown.
 */
struct chess::chess_input_error : public std::invalid_argument
{
    /* Inherit the constructor from std::invalid_argument */
    using std::invalid_argument::invalid_argument;
};

/* struct chess_internal_error
 *
 * Thrown by chess methods when an internal error has occured. The board is left in an undefined state, if this exception is thrown.
 */
struct chess::chess_internal_error : public std::runtime_error
{
    /* Inherit the constructor from std::runtime_error */
    using std::runtime_error::runtime_error;
};



/* MOVE CLASS */

/* Move class */
class chess::move_t
{
public:

    /* CONSTRUCTORS */

    /** @name  default constructor
     * 
        * @brief  Constructs an empty move
        */
    move_t () noexcept = default;

    /** @name  null move constructor
     * 
        * @brief  Constructs a null move.
        *         It will count as a null move when applied via make_move_internal or unmake_move_internal.
        *         However make_move will throw if a null move is given.
        */
    explicit move_t ( pcolor _pc ) : pc { _pc } {}

    /** @name  full constructor
     * 
        * @brief  Construct with all the requied information
        */
    move_t ( pcolor _pc, ptype _pt, ptype _capture_pt, ptype _promote_pt, int _from, int _to, bool _check = false, bool _checkmate = false, bool _stalemate = false, bool _draw = false ) noexcept
        : pc { _pc }, pt { _pt }, capture_pt { _capture_pt }, promote_pt { _promote_pt }, from { _from }, to { _to }, check { _check }, checkmate { _checkmate }, stalemate { _stalemate }, draw { _draw }
    {}



    /* OPERATORS AND COMPARISON */

    /* Default comparison operator */
    bool operator== ( const move_t& other ) const noexcept = default;

    /** @name  is_similar
     * 
     * @brief  Returns true if another move has the same pc, pt, from and to
     * @param  other: The other move.
     * @return boolean
     */
    bool is_similar ( const move_t& other ) const noexcept 
        { return pc == other.pc && pt == other.pt && from == other.from && to == other.to; }



    /* ATTRIBUTES */

    /* Store the color that moved */
    pcolor pc = pcolor::no_piece;

    /* Store the piece type that moved, the type that will be captured, and the promoted type if applicable */
    ptype pt = ptype::no_piece, capture_pt = ptype::no_piece, promote_pt = ptype::no_piece;

    /* Store the initial, final position */
    int from = 0, to = 0;

    /* Store whether the causes check, checkmate, stalemate or a draw */
    bool check = false, checkmate = false, stalemate = false, draw = false;



    /* OTHER METHODS */

    /** @name  en_passant_capture_pos
     * 
     * @brief  Assuming this is an en passant capture, get the position of the captured pawn
     *         Given by the rank of departure and the field of destination.
     * @return int
     */
    int en_passant_capture_pos () const noexcept { return ( from / 8 ) * 8 + ( to % 8 ); }

    /** @name  is_kingside/queenside_castle
     * 
     * @brief  Returns true if this move refers to a kingside or queenside castle
     * @return boolean
     */
    bool is_kingside_castle  () const noexcept { return pt == ptype::king && from + 2 == to; }
    bool is_queenside_castle () const noexcept { return pt == ptype::king && from - 2 == to; }

};



/* CHESSBOARD DEFINITION */

/* class chessboard 
 *
 * Store and manipulate a bitboard-based chessboard
 */
class chess::chessboard
{

    /* Befriend game controller class */
    friend game_controller;

public:

    /* CONSTRUCTORS */

    /** @name  default constructor
     * 
     * @brief  Sets up an opening chessboard
     */
    chessboard ();

    /** @name  copy constructor
     * 
     * @brief  Copy constructs the chessboard
     */
    chessboard ( const chessboard& other );

    /** @name  move constructor
     * 
     * @brief  Move constructs the chessboard
     */
    chessboard ( chessboard&& other ) noexcept = default;

    /** @name  copy assignment operator
     * 
     * @brief  Copy assigns the chessboard
     */
    chessboard& operator= ( const chessboard& other );

    /** @name  move assignment operator
     * 
     * @brief  Move assigns the chessboard
     */
    chessboard& operator= ( chessboard&& other ) noexcept = default;

    /** @name  destructor
     * 
     * @brief  Destructs the chessboard
     */
    ~chessboard () = default;

    /** @name  operator==
     * 
     * @brief  Compares if two chessboards are equal
     */
    bool operator== ( const chessboard& other ) const noexcept;

    /** @name  reset_to_initial
     * 
     * @brief  Resets the board and history to the default chess initial position
     * @return void
     */
    void reset_to_initial () { * this = chessboard {}; }

    /** @name  reset_to_empty
     * 
     * @brief  Resets the board and history to an empty board
     * @return void
     */
    void reset_to_empty ();



    /* AUX INFO STRUCT */

    /* Auxiliary chessboard information */
    struct aux_info_t
    {
        /* Castling rights:
         * Whether white and black is allowed to castle, based on previous moves.
         * Eight bits are used to store the rights, alternating bits for each color, in order of least to most significant:
         *   0: Castle made
         *   1: Castle lost (both)
         *   2: Can kingside castle 
         *   3: Can queenside castle
         */
        unsigned castling_rights = 0b11110000;

        /* En passant target position and color:
         * The position behind the pawn which double pushed in the previous move, and the color which can capture that pawn.
         * If there is no such pawn, the target is 0, and color is no_piece.
         * This is used to determine whether an en passant capture is allowed.
         */
        int en_passant_target = 0; pcolor en_passant_color = pcolor::no_piece;

        /* Default comparison operator */
        bool operator== ( const aux_info_t& other ) const noexcept = default;
    };



    /* CHECK INFO STRUCT */

    /* Check info */
    struct check_info_t
    {
        /* Check and pin vectors */
        bitboard check_vectors, pin_vectors;

        /* The number of times this color is in check */
        int check_count = 0;

        /* Versions of check and pin vectors but on straights and diagonals */
        bitboard straight_check_vectors, diagonal_check_vectors, straight_pin_vectors, diagonal_pin_vectors;

        /* check_vectors_dep_check_count
        * This can be intersected with possible attacks to ensure that the king was protected.
        * If not in check, the king does not need to be protected so the bitboard is set to universe.
        * If in check once, every possible move must bloock check, so the bitboard is set to check_vectors.
        * If in double check, only the king can move so the bitboard is set to empty.
        */
        bitboard check_vectors_dep_check_count;

        /* Default comparison operator */
        bool operator== ( const check_info_t& other ) const noexcept = default;
    };



    /* GAME STATE */

    /* A structure to store a state of the game */
    class game_state_t
    {
    public:

        /* CONSTRUCTORS */

        /** @name  default constructor
         * 
         * @brief  Unlike chessboard, initialized the state to be an empty board
         */
        game_state_t () noexcept = default;

        /** @name  chessboard constructor
         * 
         * @brief  Construct from a chessboard state
         * @param  cb: The chessboard to construct from
         * @param  _last_pc: The player who last moved (in order to give this state)
         */
        game_state_t ( const chessboard& cb, pcolor _last_pc ) chess_validate_throw;


        
        /* OPERATORS */

        /** @name  operator==
         * 
         * @brief  Compare two alpha-beta states
         */
        bool operator== ( const game_state_t& other ) const noexcept = default;

    

        /* BITBOARD ACCESS */

        /** @name  bb
         * 
         * @brief  Gets a bitboard, by copy, based on a piece type and or color
         * @param  pc: One of pcolor. Undefined behavior if is no_piece and validation is disabled.
         * @param  pt: One of ptype, any_piece by default, which gives all the pieces of that color. Undefined behavior if is no_piece and validation is disabled.
         * @return The bitboard for pt
         */
        bitboard bb ( pcolor pc )           const chess_validate_throw { check_penum ( pc ); return bbs [ cast_penum ( pc ) ]; }
        bitboard bb ( ptype pt )            const chess_validate_throw { check_penum ( pt ); return bbs [ cast_penum ( pt ) + 2 ]; }
        bitboard bb ( pcolor pc, ptype pt ) const chess_validate_throw { check_penum ( pc, pt ); return bbs [ cast_penum ( pc ) ] & bbs [ cast_penum ( pt ) + 2 ]; }

        /** @name  castle_made, castle_lost, has_kingside_castling_rights, has_queenside_castling_rights, has_any_castling_rights
         * 
         * @brief  Gets information about castling rights
         * @param  pc: One of pcolor. Undefined behaviour if is no_piece.
         * @return boolean
         */
        bool castle_made ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b00000001 << cast_penum ( pc ) ); }
        bool castle_lost ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b00000100 << cast_penum ( pc ) ); }
        bool has_kingside_castling_rights  ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b00010000 << cast_penum ( pc ) ); }
        bool has_queenside_castling_rights ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b01000000 << cast_penum ( pc ) ); }
        bool has_any_castling_rights       ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b01010000 << cast_penum ( pc ) ); }




        /* ATTRIBUTES */

        /* The player who last moved (to lead to this state) */
        pcolor last_pc = pcolor::no_piece;

        /* Bitboards to store the state */
        std::array<bitboard, 8> bbs;

        /* the auxiliary info */
        aux_info_t aux_info;

    };



    /* HASHING STRUCT */

    /* Struct to hash a chessboard or state */
    struct hash
    {
        /** @name  operator ()
         * 
            * @brief  Creates a hash for a chessboard
            * @param  cb: The chessboard or alpha-beta state to hash
            * @param  mv: The move to hash
            * @return The hash
            */
        std::size_t operator () ( const chessboard& cb ) const chess_validate_throw;
        std::size_t operator () ( const chessboard::game_state_t& cb ) const noexcept;
        std::size_t operator () ( const move_t& mv ) const noexcept;
    };



    /* TTABLE ENTRY STRUCT */

    /* A structure for a ttable entry in alpha-beta search */
    struct ab_ttable_entry_t
    {
        /* An enum for whether the ttable entry is exact or an upper or lower bound */
        enum class bound_t { exact, upper, lower };

        /* The value of the ttable entry */
        int value;

        /* The bk_depth that the entry was made */
        int bk_depth;

        /* The bound of the ttable entry */
        bound_t bound;

        /* The best move at this state */
        move_t best_move;
    };



    /* TTABLE TYPEDEF */

    /* The transposition table type */
    typedef std::unordered_map<game_state_t, ab_ttable_entry_t, hash> ab_ttable_t;



    /* ALPHA BETA RESULT STRUCT */

    /* Alpha beta result struct */
    struct ab_result_t
    {
        /* An array of move value pairs */
        std::vector<std::pair<move_t, int>> moves;

        /* The depth before quiescence */
        int depth = 0; 
        
        /* Number of nodes visited */
        int num_nodes = 0, num_q_nodes = 0;

        /* Average quiescence depth and moves per node */
        double av_q_depth = 0.0, av_moves = 0.0, av_q_moves = 0.0;

        /* Boolean flags for if the search was incomplete, failed low or failed high */
        bool incomplete = false, failed_low = false, failed_high = false;

        /* The time taken for the search */
        chess_clock::duration duration;

        /* The transposition table from the search */
        ab_ttable_t ttable;
    };



    /* BITBOARD ACCESS */

    /** @name  get_bb
     * 
     * @brief  Gets a bitboard, by reference, based on a single piece type
     * @param  pc: One of pcolor, undefined behaviour if no_piece and validation is disabled.
     * @param  pt: One of ptype, any_piece by default, which gives all the pieces of that color. Undefined behavior if is no_piece and validation is disabled.
     * @return The bitboard for pt 
     */
    bitboard& get_bb ( pcolor pc, ptype pt = ptype::any_piece ) chess_validate_throw { check_penum ( pc, pt ); return bbs [ cast_penum ( pt ) ] [ cast_penum ( pc ) ]; }
    const bitboard& get_bb ( pcolor pc, ptype pt = ptype::any_piece ) const chess_validate_throw { check_penum ( pc, pt ); return bbs [ cast_penum ( pt ) ] [ cast_penum ( pc ) ]; }

    /** @name  bb
     * 
     * @brief  Gets a bitboard, by copy, based on a piece type and or color
     * @param  pc: One of pcolor. Undefined behavior if is no_piece and validation is disabled.
     * @param  pt: One of ptype, any_piece by default, which gives all the pieces of that color. Undefined behavior if is no_piece and validation is disabled.
     * @return The bitboard for pt
     */
    bitboard bb ( pcolor pc, ptype pt = ptype::any_piece ) const chess_validate_throw { check_penum ( pc, pt ); return bbs [ cast_penum ( pt ) ] [ cast_penum ( pc ) ]; }
    bitboard bb ( ptype pt = ptype::any_piece ) const chess_validate_throw { check_penum ( pt ); return bbs [ cast_penum ( pt ) ] [ cast_penum ( pcolor::white ) ] | bbs [ cast_penum ( pt ) ] [ cast_penum ( pcolor::black ) ]; }
    
    /** @name  castle_made, castle_lost, has_kingside_castling_rights, has_queenside_castling_rights, has_any_castling_rights
     * 
     * @brief  Gets information about castling rights
     * @param  pc: One of pcolor. Undefined behaviour if is no_piece.
     * @return boolean
     */
    bool castle_made ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b00000001 << cast_penum ( pc ) ); }
    bool castle_lost ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b00000100 << cast_penum ( pc ) ); }
    bool has_kingside_castling_rights  ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b00010000 << cast_penum ( pc ) ); }
    bool has_queenside_castling_rights ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b01000000 << cast_penum ( pc ) ); }
    bool has_any_castling_rights       ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return aux_info.castling_rights & ( 0b01010000 << cast_penum ( pc ) ); }

    /** @name  set_castle_made, set_castle_lost, set_kingside_castle_lost, set_queenside_castle_lost 
     * 
     * @brief  Set information about castling for a color.
     * @param  pc: One of pcolor.
     * @return void
     */
    void set_castle_made ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b01010100 << cast_penum ( pc ) ); aux_info.castling_rights |= 0b00000001 << cast_penum ( pc ); }
    void set_castle_lost ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b01010001 << cast_penum ( pc ) ); aux_info.castling_rights |= 0b00000100 << cast_penum ( pc ); }
    void set_kingside_castle_lost  ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b00010000 << cast_penum ( pc ) ); if ( !has_any_castling_rights ( pc ) ) aux_info.castling_rights |= 0b00000100 << cast_penum ( pc ); }
    void set_queenside_castle_lost ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b01000000 << cast_penum ( pc ) ); if ( !has_any_castling_rights ( pc ) ) aux_info.castling_rights |= 0b00000100 << cast_penum ( pc ); }

    /** @name  get_aux_info
     * 
     * @brief  Return the current aux info
     * @return aux_info_t
     */
    aux_info_t& get_aux_info () noexcept { return aux_info; };
    const aux_info_t& get_aux_info () const noexcept { return aux_info; };

    /** @name  get_game_state
     * 
     * @brief  Create a game_state_t struct for the current board
     * @param  last_pc: The player whose who last moved (to lead to this state)
     * @return game_state_t
     */
    game_state_t get_game_state ( pcolor last_pc ) const noexcept { return game_state_t { * this, last_pc }; };



    /* BOARD EVALUATION */

    /** @name  get_check_info
     * 
     * @brief  Get information about the check state of a color's king
     * @param  pc: The color whose king we will look at
     * @return check_info_t
     */
    chess_hot check_info_t get_check_info ( pcolor pc ) const chess_validate_throw;

    /** @name  is_protected
     * 
     * @brief  Returns true if the board position is protected by the player specified.
     *         There is no restriction on what piece is at the position, since any piece in the position is ignored.
     * @param  pc: The color who is defending.
     * @param  pos: The position of the cell to check the defence of.
     * @return boolean
     */
    chess_hot bool is_protected ( pcolor pc, int pos ) const chess_validate_throw;  

    /** @name  is_in_check
     * 
     * @brief  Similar to is_protected, but considered specifically whether a king is in check
     * @param  pc: The color whose king we will look at
     * @return boolean
     */
    bool is_in_check ( pcolor pc ) const chess_validate_throw { check_penum ( pc ); return is_protected ( other_color ( pc ), bb ( pc, ptype::king ).trailing_zeros () ); }

    /** @name  can_castle, can_kingside_castle, can_queenside_castle
     * 
     * @brief  Gets information as to whether a color can legally castle given the current state
     * @param  pc: One of pcolor. Undefined behaviour if is no_piece.
     * @param  check_info: The check info for the king in its current position
     * @return boolean
     */
    bool can_kingside_castle  ( pcolor pc, const check_info_t& check_info ) const chess_validate_throw;
    bool can_queenside_castle ( pcolor pc, const check_info_t& check_info ) const chess_validate_throw;
    bool can_castle           ( pcolor pc, const check_info_t& check_info ) const chess_validate_throw { return can_kingside_castle ( pc, check_info ) | can_queenside_castle ( pc, check_info ); }

    /** @name  is_draw_state
     * 
     * @brief  Returns true if this state is a draw state by repetition.
     * @return boolean
     */
    bool is_draw_state () const noexcept { return game_state_history.size () >= 9 && game_state_history.back () == game_state_history.at ( game_state_history.size () - 5 ) && game_state_history.back () == game_state_history.at ( game_state_history.size () - 9 ); }

    /** @name  evaluate
     * 
     * @brief  Symmetrically evaluate the board state.
     * @param  pc: The color whose move it is next
     * @return Integer value, positive for pc, negative for not pc
     */
    chess_hot int evaluate ( pcolor pc );



    /* MOVE CALCULATIONS */

    /** @name  check_move_is_valid
     * 
     * @brief  Throws if a move is invalid to apply to the current state
     * @param  move: The move to test
     * @return void
     */
    void check_move_is_valid ( const move_t& move );
    void check_move_is_valid ( const move_t& move ) const { chessboard { * this }.check_move_is_valid ( move ); }

    /** @name  make_move
     * 
     * @brief  Check a move is valid then apply it
     * @param  move: The move to apply
     * @return void
     */
    void make_move ( const move_t& move ) { check_move_is_valid ( move ); make_move_internal ( move ); }

    /** @name  unmake_move
     * 
     * @brief  Unmake the last move that was made.
     * @return void.
     */
    void unmake_move () { if ( game_state_history.empty () ) throw chess_input_error { "Cannot unmake move, since game history is empty, in unmake_move ()." }; unmake_move_internal (); }

    /** @name  get_move_set
     * 
     * @brief  Gets the move set for a given type and position of piece
     * @param  pc: The color who owns the pawn
     * @param  pt: The type of the piece
     * @param  pos: The position of the pawn
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the piece in question
     */
    bitboard get_move_set ( pcolor pc, ptype pt, int pos, const check_info_t& check_info );
    bitboard get_move_set ( pcolor pc, ptype pt, int pos, const check_info_t& check_info ) const { return chessboard { * this }.get_move_set ( pc, pt, pos, check_info ); };

    /** @name  has_mobility
     * 
     * @brief  Gets whether a color has any mobility
     * @param  pc: The color to check for mobility
     * @param  check_info: The check info for pc.
     * @return boolean
     */
    bool has_mobility ( pcolor pc, const check_info_t& check_info );

    /** @name  get_pawn_move_set
     * 
     * @brief  Gets the move set for a pawn
     * @param  pc: The color who owns the pawn
     * @param  pos: The position of the pawn
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the pawn in question
     */
    bitboard get_pawn_move_set ( pcolor pc, int pos, const check_info_t& check_info );

    /** @name  get_knight_move_set
     * 
     * @brief  Gets the move set for a knight
     * @param  pc: The color who owns the knight
     * @param  pos: The position of the knight
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the knight in question
     */
    bitboard get_knight_move_set ( pcolor pc, int pos, const check_info_t& check_info );

    /** @name  get_sliding_move_set
     * 
     * @brief  Gets the move set for a sliding
     * @param  pc: The color who owns the sliding piece
     * @param  pt: The type of the sliding piece, undefined if not a sliding piece
     * @param  pos: The position of the sliding piece
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the sliding in question
     */
    bitboard get_sliding_move_set ( pcolor pc, ptype pt, int pos, const check_info_t& check_info );

    /** @name  get_king_move_set
     * 
     * @brief  Gets the move set for the king.
     * @param  pc: The color who owns the king
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the king in question
     */
    bitboard get_king_move_set ( pcolor pc, const check_info_t& check_info );



    /* SEARCH */

    /** @name  purge_ttable
     * 
     * @brief  Take a transposition table, and remove entries which are no longer reachable from the current board state.
     * @param  ttable: The transposition table to erase elements from.
     * @param  min_bk_depth: The minimum bk_depth for which an entry is allowed to stay.
     * @return A new ttable with unreachable positions erased.
     */
    ab_ttable_t purge_ttable ( ab_ttable_t ttable, int min_bk_depth = 0 ) const;

    /** @name  alpha_beta_search
     * 
     * @brief  Set up and apply the alpha-beta search.
     * @throw  If throws, the chessboard is left in an undefined state. Otherwise the state will be unmodified on return.
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
    ab_result_t alpha_beta_search ( pcolor pc, int depth, bool best_only, ab_ttable_t ttable = ab_ttable_t {}, const std::atomic_bool& end_flag = false, chess_clock::time_point end_point = chess_clock::time_point::max (), int alpha = -20000, int beta = +20000 );

    /** @name  alpha_beta_iterative_deepening
     * 
     * @brief  Apply an alpha-beta search over a range of depths.
     *         Specifying an end point could to an early return rather than starting later searches.
     * @throw  If throws, the chessboard is left in an undefined state. Otherwise the state will be unmodified on return.
     * @param  pc: The color whose move it is next.
     * @param  depths: A list of depth values to search.
     * @param  best_only: If true, the search will be optimised as only the best move is returned.
     * @param  ttable: The transposition table to use for the search. Empty by default.
     * @param  end_flag: An atomic boolean, which when set to true, will end the search. Can be unspecified.
     * @param  end_point: A time point at which the search will be automatically stopped. Never by default.
     * @param  finish_first: If true, always wait for the lowest depth search to finish, regardless of end_point or end_flag. True by default.
     * @return ab_result_t
     */
    ab_result_t alpha_beta_iterative_deepening ( pcolor pc, const std::vector<int>& depths, bool best_only, ab_ttable_t ttable = ab_ttable_t {}, const std::atomic_bool& end_flag = false,
        chess_clock::time_point end_point = chess_clock::time_point::max (), bool finish_first = true );



    /* BOARD LOOKUP */

    /** @name  find_color
     *
     * @brief  Determines the color of piece at a board position.
     * @param  pos: Board position
     * @param  rank: The rank of the position
     * @param  file: The file of the position
     * @return One of pcolor
     */
    pcolor find_color ( int pos ) const chess_validate_throw;
    pcolor find_color ( int rank, int file ) const chess_validate_throw { return find_color ( rank * 8 + file ); }

    /** @name  find_type
     * 
     * @brief  Determines the type of piece at a board position.
     * @param  pc:  The known piece color
     * @param  pos: Board position
     * @param  rank: The rank of the position
     * @param  file: The file of the position
     * @return One of ptype
     */
    ptype find_type ( pcolor pc, int pos ) const chess_validate_throw;
    ptype find_type ( pcolor pc, int rank, int file ) const chess_validate_throw { return find_type ( pc, rank * 8 + file ); }



    /* FORMATTING */

    /** @name  simple_format_board
     * 
     * @brief  Create a simple representation of the board.
     *         Lower-case letters mean black, upper case white.
     * @return string
     */
    std::string simple_format_board () const;

    /** @name  fen_serialize_board
     * 
     * @brief  Serialize the board based on Forsyth–Edwards notation
     * @param  pc: The color who's turn it is next
     * @return string
     */
    std::string fen_serialize_board ( pcolor pc ) const;

    /** @name  fen_deserialize_board
     * 
     * @brief  Deserialize a string based on Forsyth–Edwards notation, and replace this board with it.
     *         The game history will be emptied, and this state will be considered the opening state.
     * @param  desc: The board description
     * @return The color who's move it is next
     */
    pcolor fen_deserialize_board ( const std::string& desc );

    /** @name  fide_serialize_move
     * 
     * @brief  Take a move valid for this position and serialize it based on the FIDE standard
     * @param  move: The move to serialize
     * @return string
     */
    std::string fide_serialize_move ( const move_t& move ) const;

    /** @name  fide_deserialize_move
     * 
     * @brief  Take a string describing a move for this position and deserialize it based on the FIDE standard
     * @param  pc: The color that is to make the move
     * @param  desc: The description to deserialize
     * @return move_t
     */
    move_t fide_deserialize_move ( pcolor pc, const std::string& desc ) const;



private:

    /* TYPES */

    /* A structure containing temporary alpha-beta search data */
    struct ab_working_t
    {
        /* Accumulate the sum of quiescence depth and moves made */
        unsigned long long sum_q_depth = 0, sum_moves = 0, sum_q_moves = 0;
        
        /* Accumulate the number of full nodes and quiescence nodes visited */
        int num_nodes = 0, num_q_nodes = 0;

        /* The largest fd_depth for which a draw state could occur */
        int draw_max_fd_depth = 0;

        /* Array of sets of moves */
        std::vector<std::array<std::vector<std::pair<int, bitboard>>, 6>> move_sets;

        /* An array of root moves and their values */
        std::vector<std::pair<move_t, int>> root_moves;

        /* An array of the two most recent killer moves for each depth */
        std::vector<std::array<move_t, 2>> killer_moves;

        /* The transposition table */
        std::unordered_map<game_state_t, ab_ttable_entry_t, hash> ttable;
    };



    /* ATTRIBUTES */

    /* 2D array of type and color bitboards */
    std::array<std::array<bitboard, 2>, 7> bbs
    { 
        bitboard { 0x000000000000ff00 }, bitboard { 0x00ff000000000000 },
        bitboard { 0x0000000000000042 }, bitboard { 0x4200000000000000 },
        bitboard { 0x0000000000000024 }, bitboard { 0x2400000000000000 },
        bitboard { 0x0000000000000081 }, bitboard { 0x8100000000000000 },
        bitboard { 0x0000000000000008 }, bitboard { 0x0800000000000000 },
        bitboard { 0x0000000000000010 }, bitboard { 0x1000000000000000 },
        bitboard { 0x000000000000ffff }, bitboard { 0xffff000000000000 }
    };

    /* Auxiliary information */
    aux_info_t aux_info;

    /* The game state history */
    std::vector<game_state_t> game_state_history;

    /* A structure containing temporary alpha-beta search data */
    mutable std::unique_ptr<ab_working_t> ab_working;



    /* STATIC ATTRIBUTES */

    /* The characters used for pieces based on ptype */
    static constexpr char piece_chars [] = "PNBRQK#.";



    /* MAKING AND UNMAKING MOVES */

    /** @name  make_move_internal
     * 
     * @brief  Apply a move. Assumes all the information about the move is correct and legal.
     * @param  move: The move to apply
     * @return void
     */
    void make_move_internal ( const move_t& move );

    /** @name  unmake_move
     * 
     * @brief  Unmake the last made move.
     * @return void
     */
    void unmake_move_internal ();

    /** @name  ptype_to_character
     * 
     * @brief  Convert a ptype to a character
     * @param  pt: The character to convert
     * @return char
     */
    static char ptype_to_character ( ptype pt ) noexcept;

    /** @name  character_to_ptype
     * 
     * @brief  Convert a character to a ptype. Return no_piece for an unknown character.
     * @param  c: The character to convert
     * @return ptype
     */
    static ptype character_to_ptype ( char c ) noexcept;



    /* SEARCH */

    /** @name  alpha_beta_search_internal
     * 
     * @brief  Apply an alpha-beta search to a given depth.
     *         Note that although is non-const, a call to this function which does not throw will leave the object unmodified.
     * @param  pc: The color whose move it is next.
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
    chess_hot int alpha_beta_search_internal ( pcolor pc, int bk_depth, bool best_only, const std::atomic_bool& end_flag = false, chess_clock::time_point end_point = chess_clock::time_point::max (),
        int alpha        = -20000, 
        int beta         = +20000, 
        int fd_depth     = 0, 
        int null_depth   = 0, 
        int q_depth      = 0
    );



    /* SANITY CHECKS */

    /** @name  sanity_check_bbs
     * 
     * @brief  Sanity check the bitboards describing the board state. 
     *         If any cell is occupied by multiple pieces, or ptype::any_piece bitboards are not correct, an exception is thrown.
     *         Does nothing if CHESS_VALIDATE is not set to true.
     * @param  _last_pc: The player who last moved
     * @return void
     */
    void sanity_check_bbs ( pcolor _last_pc ) const chess_validate_throw;

};



/* INCLUDE INLINE IMPLEMENTATION */
#include <chess/chessboard.hpp>



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */