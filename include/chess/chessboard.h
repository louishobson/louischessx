/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 * 
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 * 
 * include/chess/bitboard.h
 * 
 * Header file for managing a chess bitboard
 * 
 * NOT IMPLEMENTED 
 * 
 * https://www.chessprogramming.org/Defended_Pawns_(Bitboards)
 * Rest of pawns
 * 
 * 
 * 
 */



/* HEADER GUARD */
#ifndef CHESSBOARD_H_INCLUDED
#define CHESSBOARD_H_INCLUDED



/* INCLUDES */
#include <algorithm>
#include <chess/bitboard.h>
#include <chess/macros.h>
#include <iostream>
#include <future>
#include <unordered_map>
#include <utility>
#include <string>
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



    /* class chessboard 
     *
     * Store and manipulate a bitboard-based chessboard
     */
    class chessboard;
}



/* CHESSBOARD DEFINITION */

/* class chessboard 
 *
 * Store and manipulate a bitboard-based chessboard
 */
class chess::chessboard
{

public:

    /* CONSTRUCTORS */

    /** @name  default constructor
     * 
     * @brief  Sets up an opening chessboard
     */
    chessboard () noexcept = default;

    /** @name  copy constructor
     * 
     * @brief  Copy constructs the chessboard
     */
    chessboard ( const chessboard& other ) noexcept;

    /** @name  move constructor
     * 
     * @brief  Differs from the copy constructor only by ab_working being moved, rather than ignored.
     */
    chessboard ( chessboard&& other ) noexcept = default;

    /** @name  copy assignment operator
     * 
     * @brief  Copy assigns the chessboard
     */
    chessboard& operator= ( const chessboard& other ) noexcept;

    /** @name  move assignment operator
     * 
     * @brief  Differs from the copy assignment only by ab_working being moved, rather than ignored.
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



    /* MOVE CLASS */

    /* Move class */
    class move_t
    {
    public:

        /* CONSTRUCTORS */

        /** @name  default constructor
         * 
         * @brief  Constructs an empty move, or a null move.
         *         It will count as a null move when applied via make_move_internal or unmake_move_internal.
         *         However make_move will throw if a null move is given.
         */
        move_t () noexcept = default;

        /** @name  full constructor
         * 
         * @brief  Construct with all the requied information
         */
        move_t ( pcolor _pc, ptype _pt, ptype _capture_pt, ptype _promote_pt, int _from, int _to, int _en_passant_pos, int _check_count ) noexcept
            : pc { _pc }, pt { _pt }, capture_pt { _capture_pt }, promote_pt { _promote_pt }, from { _from }, to { _to }, en_passant_pos { _en_passant_pos }, check_count { _check_count }
        {}

        /** @name  deserialize constructor
         * 
         * @brief  Take a serialized move and deserialize it
         * @param  pc: The color who is making the move
         * @param  desc: The serialized move
         */
        move_t ( pcolor pc, const std::string& desc ) { deserialize ( pc, desc ); }



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

        /* Store the initial, final position, en passant position (if applicable; zero means no en passant), and the check count */
        int from = 0, to = 0, en_passant_pos = 0, check_count = 0;



        /* OTHER METHODS */

        /** @name  is_kingside/queenside_castle
         * 
         * @brief  Returns true if this move refers to a kingside or queenside castle
         * @return boolean
         */
        bool is_kingside_castle  () const noexcept { return pt == ptype::king && from + 2 == to; }
        bool is_queenside_castle () const noexcept { return pt == ptype::king && from - 2 == to; }

        /** @name  serialize
         * 
         * @brief  Creates a string from the move
         * @return string
         */
        std::string serialize () const;

        /** @name  deserialize
         * 
         * @brief  Sets the move from a string
         * @param  _pc: The color that is to make the move
         * @param  desc: The serialization of the move
         * @return A reference to this move
         */
        move_t& deserialize ( pcolor _pc, const std::string& desc );

    };



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
        int castling_rights = 0b11110000;

        /* Double push position:
         * The position of the pawn which double pushed in the previous move.
         * If there is no such pawn, the value is 0.
         * This is used to determine whether an en passant capture is allowed.
         */
        int double_push_pos = 0;

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



    /* ALPHA BETA STATE */

    /* A structure to store a state of alpha-beta search */
    class ab_state_t
    {
    public:

        /* CONSTRUCTORS */

        /** @name  default constructor
         * 
         * @brief  Unlike chessboard, initialized the state to be an empty board
         */
        ab_state_t () noexcept = default;

        /** @name  chessboard constructor
         * 
         * @brief  Construct from a chessboard state
         * @param  cb: The chessboard to construct from
         * @param  _pc: The player whose move it is next
         */
        ab_state_t ( const chessboard& cb, pcolor _pc ) chess_validate_throw;


        
        /* OPERATORS */

        /** @name  operator==
         * 
         * @brief  Compare two alpha-beta states
         */
        bool operator== ( const ab_state_t& other ) const noexcept = default;

        

        /* ATTRIBUTES */

        /* Which player's turn it is */
        pcolor pc;

        /* Bitboards to store the state */
        std::array<bitboard, 8> bbs;

        /* the auxiliary info */
        aux_info_t aux_info;

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
        std::size_t operator () ( const ab_state_t& cb ) const noexcept;
        std::size_t operator () ( const move_t& mv ) const noexcept;
    };



    /* WORKING VALUES STRUCT */

    /* A structure containing temporary alpha-beta search data */
    struct ab_working_t
    {
        /* Accumulate the sum of quiescence depth and moves made */
        unsigned long long sum_q_depth = 0, sum_moves = 0, sum_q_moves = 0;
        
        /* Accumulate the number of full nodes and quiescence nodes visited */
        int num_nodes = 0, num_q_nodes = 0;

        /* Array of sets of moves */
        std::vector<std::array<std::vector<std::pair<int, bitboard>>, 6>> move_sets;

        /* An array of root moves and their values */
        std::vector<std::pair<move_t, int>> root_moves;

        /* An array of the two most recent killer moves for each depth */
        std::vector<std::array<move_t, 2>> killer_moves;

        /* The transposition table */
        std::unordered_map<ab_state_t, ab_ttable_entry_t, hash> ttable;
    };



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

        /* The time multiple between this depth search and the previous when using iterative deepening */
        double time_multiple = 0.0;

        /* The time taken for the search */
        std::chrono::system_clock::duration duration;

        /* Alpha beta working values from the search */
        std::unique_ptr<ab_working_t> _ab_working;
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
     * @brief  Gets a bitboard, by copy, based on a single piece type
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
    void set_castle_made ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b01010000 << cast_penum ( pc ) ); aux_info.castling_rights |= 0b00000001 << cast_penum ( pc ); }
    void set_castle_lost ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b01010000 << cast_penum ( pc ) ); aux_info.castling_rights |= 0b00000100 << cast_penum ( pc ); }
    void set_kingside_castle_lost  ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b00010000 << cast_penum ( pc ) ); if ( !has_any_castling_rights ( pc ) ) aux_info.castling_rights |= 0b00000100 << cast_penum ( pc ); }
    void set_queenside_castle_lost ( pcolor pc ) chess_validate_throw { check_penum ( pc ); aux_info.castling_rights &= ~( 0b01000000 << cast_penum ( pc ) ); if ( !has_any_castling_rights ( pc ) ) aux_info.castling_rights |= 0b00000100 << cast_penum ( pc ); }



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

    /** @name  evaluate
     * 
     * @brief  Symmetrically evaluate the board state.
     *         Note that although is non-const, a call to this function will leave the board unmodified.
     * @param  pc: The color whose move it is next
     * @return Integer value, positive for pc, negative for not pc
     */
    chess_hot int evaluate ( pcolor pc ) chess_validate_throw;



    /* MOVE CALCULATIONS */

    /** @name  make_move
     * 
     * @brief  Check a move is valid then apply it
     * @param  move: The move to apply
     * @return void
     */
    void make_move ( const move_t& move );

    /** @name  get_move_set
     * 
     * @brief  Gets the move set for a given type and position of piece
     * @param  pc: The color who owns the pawn
     * @param  pt: The type of the piece
     * @param  pos: The position of the pawn
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the piece in question
     */
    bitboard get_move_set ( pcolor pc, ptype pt, int pos, const check_info_t& check_info ) chess_validate_throw;

    /** @name  get_pawn_move_set
     * 
     * @brief  Gets the move set for a pawn
     * @param  pc: The color who owns the pawn
     * @param  pos: The position of the pawn
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the pawn in question
     */
    bitboard get_pawn_move_set ( pcolor pc, int pos, const check_info_t& check_info ) chess_validate_throw;

    /** @name  get_knight_move_set
     * 
     * @brief  Gets the move set for a knight
     * @param  pc: The color who owns the knight
     * @param  pos: The position of the knight
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the knight in question
     */
    bitboard get_knight_move_set ( pcolor pc, int pos, const check_info_t& check_info ) chess_validate_throw;

    /** @name  get_sliding_move_set
     * 
     * @brief  Gets the move set for a sliding
     * @param  pc: The color who owns the sliding piece
     * @param  pt: The type of the sliding piece, undefined if not a sliding piece
     * @param  pos: The position of the sliding piece
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the sliding in question
     */
    bitboard get_sliding_move_set ( pcolor pc, ptype pt, int pos, const check_info_t& check_info ) chess_validate_throw;

    /** @name  get_king_move_set
     * 
     * @brief  Gets the move set for the king.
     * @param  pc: The color who owns the king
     * @param  check_info: The check info for pc
     * @return A bitboard containing the possible moves for the king in question
     */
    bitboard get_king_move_set ( pcolor pc, const check_info_t& check_info ) chess_validate_throw;



    /* SEARCH */

    /** @name  alpha_beta_search
     * 
     * @brief  Set up and apply the alpha-beta search asynchronously.
     *         The board state is saved before return, so may be safely modified after returning but before resolution of the future.
     * @param  pc: The color whose move it is next.
     * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
     * @param  end_flag: An atomic boolean, which when set to true, will end the search. Can be unspecified.
     * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
     * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
     * @return A future to an ab_result_t struct
     */
    std::future<chess::chessboard::ab_result_t> alpha_beta_search ( pcolor pc, int depth, const std::atomic_bool& end_flag = false, int alpha = -20000, int beta = +20000 ) const;

    /** @name  alpha_beta_iterative_deepening
     * 
     * @brief  Apply an alpha-beta search over a range of depths asynchronously.
     *         The board state is saved before return, so may be safely modified after returning but before resolution of the future.
     *         Specifying an end point with a depth range of at least 4 could to an early return before the 4th search has finished.
     *         The method will predict the time for the 4rd and beyond depth using an exponential model, and return if will exceed the end point.
     * @param  pc: The color whose move it is next.
     * @param  min_depth: The lower bound of the depths to try.
     * @param  max_depth: The upper bound of the depths to try.
     * @param  end_flag: An atomic boolean, which when set to true, will end the search. Can be unspecified.
     * @param  end_point: A time point at which the search will be automatically stopped. Never by default.
     * @param  finish_first: If true, always wait for the lowest depth search to finish, regardless of end_point or end_flag. True by default.
     * @return A future to an ab_result_t struct.
     */
    std::future<ab_result_t> alpha_beta_iterative_deepening ( pcolor pc, int min_depth, int max_depth, std::atomic_bool& end_flag, 
        std::chrono::system_clock::time_point end_point = std::chrono::system_clock::time_point::max (), 
        bool finish_first = true 
    ) const;



    /* BOARD LOOKUP */

    /** @name  find_color
     *
     * @brief  Determines the color of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pos: Board position
     * @return One of pcolor
     */
    pcolor find_color ( int pos ) const chess_validate_throw;

    /** @name  find_type
     * 
     * @brief  Determines the type of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pc:  The known piece color
     * @param  pos: Board position
     * @param  pos_bb: Singleton bitboard
     * @return One of ptype
     */
    ptype find_type ( pcolor pc, int pos ) const chess_validate_throw;
    ptype find_type ( pcolor pc, bitboard pos_bb ) const chess_validate_throw;



    /* FORMATTING */

    /** @name  simple_format_board
     * 
     * @brief  Create a simple representation of the board.
     *         Lower-case letters mean black, upper case white.
     * @return string
     */
    std::string simple_format_board () const;



private:

    /* PRIVATE TYPES */



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
     * @return The auxiliary information before the move was made
     */
    aux_info_t make_move_internal ( const move_t& move ) chess_validate_throw;

    /** @name  unmake_move
     * 
     * @brief  Unmake a move. Assumes that the move was made immediately before this undo function.
     * @param  move: The move to undo
     * @param  aux: The auxiliary information from before the move was first made
     * @return void
     */
    void unmake_move_internal ( const move_t& move, aux_info_t aux_info ) chess_validate_throw;

    /* SEARCH */

    /** @name  alpha_beta_search_internal
     * 
     * @brief  Apply an alpha-beta search to a given depth.
     *         Note that although is non-const, a call to this function which does not throw will leave the object unmodified.
     * @param  pc: The color whose move it is next
     * @param  bk_depth: The backwards depth, or the number of moves left before quiescence search
     * @param  end_flag: An atomic boolean, which when set to true, will end the search.
     * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
     * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
     * @param  fd_depth: The forwards depth, or the number of moves since the root node, 0 by default.
     * @param  null_depth: The null depth, or the number of nodes that null move search has been active for, 0 by default.
     * @param  q_depth: The quiescence depth, or the number of nodes that quiescence search has been active for, 0 by default.
     * @param  null_window: Whether a null window has been set, false by default.
     * @return alpha_beta_t
     */
    chess_hot int alpha_beta_search_internal ( pcolor pc, int bk_depth, const std::atomic_bool& end_flag = false,
        int alpha        = -20000, 
        int beta         = +20000, 
        int fd_depth     = 0, 
        int null_depth   = 0, 
        int q_depth      = 0,
        bool null_window = false
    );



    /* SANITY CHECKS */

    /** @name  sanity_check_bbs
     * 
     * @brief  Sanity check the bitboards describing the board state. 
     *         If any cell is occupied by multiple pieces, or ptype::any_piece bitboards are not correct, an exception is thrown.
     *         Does nothing if CHESS_VALIDATE is not set to true.
     * @return void
     */
    void sanity_check_bbs () const chess_validate_throw;

};



/* INCLUDE INLINE IMPLEMENTATION */
#include <chess/chessboard.hpp>
#include <chess/chessboard_moves.hpp>



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */