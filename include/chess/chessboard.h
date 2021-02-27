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
        queen,
        rook,
        bishop,
        knight,
        pawn,
        king,
        no_piece
    };

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

    /** @name  copy assignment operator
     * 
     * @brief  Copy assigns the chessboard
     */
    chessboard& operator= ( const chessboard& other ) noexcept;

    /** @name  destructor
     * 
     * @brief  Destructs the chessboard
     */
    ~chessboard () noexcept;

    /** @name  operator==
     * 
     * @brief  Compares if two chessboards are equal
     */
    bool operator== ( const chessboard& other ) const noexcept;



    /* PUBLIC TYPES */

    /* Check info */
    struct check_info_t
    {
        /* Check and block vectors */
        bitboard check_vectors;
        bitboard block_vectors;
    };

    /* Move struct */
    struct move_t
    {
        /* Store the color that moved */
        pcolor pc = pcolor::no_piece;

        /* Store the piece type that moved */
        ptype pt = ptype::no_piece;

        /* Store the initial and final positions in bitboards */
        bitboard from_bb, to_bb;

        /* Default comparison operator */
        bool operator== ( const move_t& other ) const noexcept = default;
    };

    /* Typedef for a list of moves and their values */
    typedef std::vector<std::pair<move_t, int>> move_list_t;



    /* BITBOARD ACCESS */

    /** @name  get_bb
     * 
     * @brief  Gets a bitboard, by reference, based on a single piece type
     * @param  pc: One of pcolor, undefined behaviour if no_piece.
     * @param  pt: One of ptype, no_piece by default, which gives all the pieces of that color.
     * @return The bitboard for pt 
     */
    bitboard& get_bb ( pcolor pc, ptype pt = ptype::no_piece ) noexcept { return bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pc ) ]; }
    const bitboard& get_bb ( pcolor pc, ptype pt = ptype::no_piece ) const noexcept { return bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pc ) ]; }

    /** @name  bb
     * 
     * @brief  Gets a bitboard, by copy, based on a single piece type
     * @param  pc: One of pcolor. Undefined behavior if is no_piece.
     * @param  pt: One of ptype, no_piece by default, which gives all the pieces of that color.
     * @return The bitboard for pt
     */
    bitboard bb ( pcolor pc, ptype pt = ptype::no_piece ) const noexcept { return bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pc ) ]; }
    bitboard bb ( ptype pt = ptype::no_piece ) const noexcept { return bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pcolor::white ) ] | bbs [ static_cast<int> ( pt ) ] [ static_cast<int> ( pcolor::black ) ]; }
    
    /** @name  can_castle, castle_made, castle_lost
     * 
     * @brief  Gets information about castling for each color.
     *         castle_lost gives whether both kingside and castling rights have been lost.
     * @param  pc: One of pcolor. Undefined behaviour if is no_piece.
     * @return boolean
     */
    bool can_castle  ( pcolor pc ) const noexcept { return castling_rights & ( 0b01010000 << static_cast<int> ( pc ) ); }
    bool castle_made ( pcolor pc ) const noexcept { return castling_rights & ( 0b00000001 << static_cast<int> ( pc ) ); }
    bool castle_lost ( pcolor pc ) const noexcept { return castling_rights & ( 0b00000100 << static_cast<int> ( pc ) ); }



    /* BOARD EVALUATION */

    /** @name  get_check_info
     * 
     * @brief  Get information about the check state of a color's king
     * @param  pc: The color who's king we will look at
     * @return check_info_t
     */
    chess_hot check_info_t get_check_info ( pcolor pc ) const;

    /** @name  is_protected
     * 
     * @brief  Returns true if the board position is protected by the player specified.
     *         There is no restriction on what piece is at the position, since any piece in the position is ignored.
     * @param  pc: The color who is defending.
     * @param  pos: The position of the cell to check the defence of.
     * @return boolean
     */
    chess_hot bool is_protected ( pcolor pc, unsigned pos ) const noexcept;  

    /** @name  is_in_check
     * 
     * @brief  Similar to is_protected, but considered specifically whether a king is in check
     * @param  pc: The color who's king we will look at
     * @return boolean
     */
    bool is_in_check ( pcolor pc ) const noexcept { return is_protected ( other_color ( pc ), bb ( pc, ptype::king ).trailing_zeros_nocheck () ); }

    /** @name  evaluate
     * 
     * @brief  Symmetrically evaluate the board state.
     *         Note that although is non-const, a call to this function will leave the board unmodified.
     * @param  pc: The color who's move it is next
     * @return Integer value, positive for pc, negative for not pc
     */
    chess_hot int evaluate ( pcolor pc );



    /* SEARCH */

    /** @name  alpha_beta_search
     * 
     * @brief  Set up and apply the alpha beta search
     * @param  pc: The color who's move it is next
     * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
     * @param  end_point: The time point at which the search should be ended, never by default.
     * @return An array of moves and their values
     */
    move_list_t alpha_beta_search ( pcolor pc, unsigned depth, std::chrono::steady_clock::time_point end_point = std::chrono::steady_clock::time_point::max () );

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
    move_list_t alpha_beta_iterative_deepening ( pcolor pc, unsigned min_depth, unsigned max_depth, std::chrono::steady_clock::time_point end_point, unsigned threads = 0 );



    /* BOARD LOOKUP */

    /** @name  find_color
     *
     * @brief  Determines the color of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pos: Board position
     * @return One of pcolor
     */
    pcolor find_color ( unsigned pos ) const noexcept;

    /** @name  find_type
     * 
     * @brief  Determines the type of piece at a board position.
     *         Note: an out of range position leads to undefined behavior.
     * @param  pc:  The known piece color
     * @param  pos: Board position
     * @param  pos_bb: Singleton bitboard
     * @return One of ptype
     */
    ptype find_type ( pcolor pc, unsigned pos ) const noexcept;
    ptype find_type ( pcolor pc, bitboard pos_bb ) const noexcept;



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

    /* A structure to store a state of alpha-beta search.
     * Defined after chessboard, since is non-trivial and would be clumbersome to define here.
     */
    struct ab_state_t;

    /* A structure containing temporary alpha-beta search data.
     * Defined after chessboard, since depends on chessboard being complete.
     */
    struct ab_working_t;

    /* HASHING STRUCT */

    /* struct hash
     *
     * Creates a hash for the chessboard
     */
    struct hash
    {
        /** @name  operator ()
         * 
         * @brief  Creates a hash for a chessboard
         * @param  cb: The chessboard or alpha-beta state to hash
         * @param  mv: The move to hash
         * @return The hash
         */
        std::size_t operator () ( const chessboard& cb ) const noexcept;
        std::size_t operator () ( const ab_state_t& cb ) const noexcept;
        std::size_t operator () ( const move_t& mv ) const noexcept;
    };



    /* ATTRIBUTES */

    /* 2D array of type and color bitboards */
    std::array<std::array<bitboard, 2>, 7> bbs
    { 
        bitboard { 0x0000000000000010 }, bitboard { 0x0800000000000000 },
        bitboard { 0x0000000000000081 }, bitboard { 0x8100000000000000 },
        bitboard { 0x0000000000000024 }, bitboard { 0x2400000000000000 },
        bitboard { 0x0000000000000042 }, bitboard { 0x4200000000000000 },
        bitboard { 0x000000000000ff00 }, bitboard { 0x00ff000000000000 },
        bitboard { 0x0000000000000008 }, bitboard { 0x1000000000000000 },
        bitboard { 0x000000000000ffff }, bitboard { 0xffff000000000000 }
    };

    /* Whether white and black has castling rights.
     * Only records whether the king or rooks have been moved, not whether castling is at this time possible.
     * Eight bits are used to store the rights, alternating bits for each color, in order of least to most significant:
     *   0: Castle made
     *   1: Castle lost (both)
     *   2: Can kingside castle 
     *   3: Can queenside castle
     */
    unsigned castling_rights = 0;

    /* A structure containing temporary alpha beta search data */
    ab_working_t * ab_working = nullptr;



    /* SEARCH */

    /** @name  alpha_beta_search_internal
     * 
     * @brief  Apply an alpha-beta search to a given depth.
     *         Note that although is non-const, a call to this function which does not throw will leave the object unmodified.
     * @param  pc: The color who's move it is next
     * @param  depth: The number of moves that should be made by individual colors. Returns evaluate () at depth = 0.
     * @param  end_point: The time point at which the search should be ended, never by default.
     * @param  fd_depth: The forwards depth, defaults to 0 and should always be 0.
     * @param  alpha: The maximum value pc has discovered, defaults to an abitrarily large negative integer.
     * @param  beta:  The minimum value not pc has discovered, defaults to an abitrarily large positive integer.
     * @param  has_null: Whether a null move has previously been applied, defaults to false.
     * @param  quiesce: Whether quiescence has started, defaults to false.
     * @return alpha_beta_t
     */
    chess_hot int alpha_beta_search_internal ( pcolor pc, unsigned depth, 
        std::chrono::steady_clock::time_point end_point = std::chrono::steady_clock::time_point::max (), 
        unsigned fd_depth = 0, 
        int alpha         = std::numeric_limits<int>::min () + 1, 
        int beta          = std::numeric_limits<int>::max (),
        bool has_null     = false, 
        bool quiesce      = false 
    );



    /* ATTACK LOOKUPS */

    /** @name  any_attack_lookup
     * 
     * @brief  lookup an attack set based on a type
     * @param  pt: The type of the piece
     * @param  pos: The position of the piece
     * @return A bitboard for the attack set
     */
    constexpr bitboard any_attack_lookup ( ptype pt, unsigned pos ) const noexcept;

};



/* AB_STATE_T DEFINITION */

/* A structure to store a state of alpha-beta search.
 * Defined after chessboard, since is non-trivial and would be clumbersome to define here.
 */
struct chess::chessboard::ab_state_t
{

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
     * @param  pc: The player who's move it is next
     */
    ab_state_t ( const chessboard& cb, pcolor pc ) noexcept;

    /** @name  operator==
     * 
     * @brief  Compare two alpha-beta states
     */
    bool operator== ( const ab_state_t& other ) const noexcept = default;

    

    /* ATTRIBUTES */

    /* Bitboards to store the state */
    std::array<bitboard, 8> bbs;

    /* Store the next player's turn and check info */
    unsigned pc_and_check_info = 0;
    
};



/* AB_WORKING_T DEFINITION */

/* A structure containing temporary alpha-beta search data */
struct chess::chessboard::ab_working_t
{
    /* Array of sets of moves */
    std::vector<std::array<std::vector<std::pair<bitboard, bitboard>>, 6>> moves;

    /* An array of root moves and their values */
    std::vector<std::pair<move_t, int>> root_moves;

    /* An array of the two most recent killer moves for each depth */
    std::vector<std::pair<move_t, move_t>> killer_moves;

    /* The transposition table */
    std::unordered_map<ab_state_t, int, hash> ttable;
};



/* INCLUDE INLINE IMPLEMENTATION */
#include <chess/chessboard.hpp>



/* HEADER GUARD */
#endif /* #ifndef CHESSBOARD_H_INCLUDED */