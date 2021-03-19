/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * include/chess/chessboard.hpp
 *
 * Inline implementation of general functions in include/chess/chessboard.h
 *
 */



/* HEADER GUARD */
#ifndef CHESS_CHESSBOARD_HPP_INCLUDED
#define CHESS_CHESSBOARD_HPP_INCLUDED



/* INCLUDES */
#include <chess/chessboard.h>



/* PIECE ENUMS */



/** @name  bool_color
 * 
 * @brief  cast a piece color to a bool
 * @param  pc: The piece color to cast. Undefined behavior if is no_piece.
 * @return bool
 */
inline constexpr bool chess::bool_color ( const pcolor pc ) noexcept {  return static_cast<bool> ( pc ); }

/** @name  other_color
 * 
 * @brief  Take a piece color and give the other color
 * @param  pc: The piece color to switch. Undefined behavior if is no_piece.
 * @return The other color of piece
 */
inline constexpr chess::pcolor chess::other_color ( const pcolor pc ) noexcept { return static_cast<pcolor> ( !static_cast<bool> ( pc ) ); }

/** @name  cast_penum
 * 
 * @brief  Casts a penum to its underlying type
 * @param  pc: The piece color to cast
 * @param  pt: The piece type to cast
 * @return int
 */
inline constexpr int chess::cast_penum ( pcolor pc ) noexcept { return static_cast<int> ( pc ); }
inline constexpr int chess::cast_penum ( ptype  pt ) noexcept { return static_cast<int> ( pt ); }

/** @name  check_penum
 * 
 * @brief  This function does nothing if validation is disabled.
 *         Otherwise, it takes a pc and pt to be used to access the bbs array, and validate that they are possible.
 * @param  pc: The piece color
 * @param  pt: The piece type. Defaults to any_piece (which will pass).
 * @return void, but will throw if validation is enabled and pc or pt are out of range.
 */
inline void chess::check_penum ( const pcolor pc, const ptype pt ) chess_validate_throw
{
    /* Only if validation is enabled */
#if CHESS_VALIDATE
    if ( pc == pcolor::no_piece ) throw std::runtime_error { "Recieved a piece color of no_piece where no_piece is not acceptable" };
    if ( pt == ptype::no_piece  ) throw std::runtime_error { "Recieved a piece type of no_piece where no_piece is not acceptable" };
#endif
}
inline void chess::check_penum ( const ptype pt ) chess_validate_throw
{
    /* Only if validation is enabled */
#if CHESS_VALIDATE
    if ( pt == ptype::no_piece  ) throw std::runtime_error { "Recieved a piece type of no_piece where no_piece is not acceptable" };
#endif
}



/* CHESSBOARD CONSTRUCTORS AND OPERATORS */



/** @name  default constructor
 * 
 * @brief  Sets up an opening chessboard
 */
inline chess::chessboard::chessboard ()
    /* Default initialize all non-static attributes */
{
    /* Set the initial history */
    game_state_history.push_back ( get_game_state ( pcolor::white ) ); 
}

/** @name  copy constructor
 * 
 * @brief  Copy constructs the chess board
 */
inline chess::chessboard::chessboard ( const chessboard& other )
    /* Initialize values */
    : bbs                { other.bbs }
    , aux_info           { other.aux_info }
    , game_state_history { other.game_state_history }

    /* Don't create ab_working, since it will be created if a search occurs */
    , ab_working { nullptr }
{}

/** @name  copy assignment operator
 * 
 * @brief  Copy assigns the chess board
 */
inline chess::chessboard& chess::chessboard::operator= ( const chessboard& other )
{
    /* Copy over values */
    bbs                = other.bbs;
    aux_info           = other.aux_info;
    game_state_history = other.game_state_history;

    /* Return this object */
    return * this;
}

/** @name  operator==
 * 
 * @brief  Compares if two chessboards are equal
 */
inline bool chess::chessboard::operator== ( const chessboard& other ) const noexcept
{
    /* Compare and return. The games do not have to have the same history. */
    return ( ( bbs == other.bbs ) && ( aux_info.castling_rights == other.aux_info.castling_rights ) && ( aux_info.castling_rights == other.aux_info.double_push_pos ) );
}

/** @name  copy_with_ab_working
 * 
 * @brief  Produce a copy of this chessboard, however with ab_working copied to the copy.
 * @return chessboard
 */
inline chess::chessboard chess::chessboard::copy_with_ab_working () const
{
    /* Make a copy */
    chessboard cb { * this };

    /* Copy ab_working */
    if ( ab_working ) cb.ab_working.reset ( new ab_working_t { * ab_working } );

    /* Return cb */
    return cb;
}

/** @name  copy_and_move_ab_working
 * 
 * @brief  Produce a copy of this chessboard, however with ab_working moves to the copy.
 * @return chessboard
 */
inline chess::chessboard chess::chessboard::copy_and_move_ab_working () const
{
    /* Make a copy */
    chessboard cb { * this };

    /* Move ab_working */
    cb.ab_working = std::move ( ab_working );

    /* Return cb */
    return cb;
}



/* FIND COLOR AND TYPE */



/** @name  find_color
 *
 * @brief  Determines the color of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return One of pcolor
 */
inline chess::pcolor chess::chessboard::find_color ( const int pos ) const chess_validate_throw
{
    if ( bb ( pcolor::white ).test ( pos ) ) return pcolor::white; else
    if ( bb ( pcolor::black ).test ( pos ) ) return pcolor::black; else
    return pcolor::no_piece;
}

/** @name  find_type
 * 
 * @brief  Determines the type of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pc:  The known piece color
 * @param  pos: Board position
 * @return One of ptype
 */
inline chess::ptype chess::chessboard::find_type ( const pcolor pc, const int pos ) const chess_validate_throw
{
    /* First detect if there is a piece, then find what type it is */
    if ( !bb ( pc ).test ( pos ) ) return ptype::no_piece;
    if ( bb ( pc, ptype::pawn   ).test ( pos ) ) return ptype::pawn;   else
    if ( bb ( pc, ptype::knight ).test ( pos ) ) return ptype::knight; else
    if ( bb ( pc, ptype::bishop ).test ( pos ) ) return ptype::bishop; else
    if ( bb ( pc, ptype::rook   ).test ( pos ) ) return ptype::rook;   else
    if ( bb ( pc, ptype::queen  ).test ( pos ) ) return ptype::queen;  else
    if ( bb ( pc, ptype::king   ).test ( pos ) ) return ptype::king;   else
    return ptype::no_piece;
}



/* AB_STATE_T IMPLEMENTATION */



/** @name  chessboard constructor
 * 
 * @brief  Construct from a chessboard state
 * @param  cb: The chessboard to construct from
 * @param  _pc: The player who's move it is next
 */
inline chess::chessboard::game_state_t::game_state_t ( const chessboard& cb, pcolor _pc ) chess_validate_throw : pc ( _pc ), bbs 
{
    cb.bb ( ptype::pawn   ),
    cb.bb ( ptype::knight ),
    cb.bb ( ptype::bishop ),
    cb.bb ( ptype::rook   ),
    cb.bb ( ptype::queen  ),
    cb.bb ( ptype::king   ),
    cb.bb ( pcolor::white ),
    cb.bb ( pcolor::black ),
}, aux_info { cb.aux_info } {}



/* HASHING FUNCTION IMPLEMENTATION */



/** @name  operator ()
 * 
 * @brief  Creates a hash for a chessboard
 * @param  cb: The chessboard or alpha-beta state to hash
 * @param  mv: The move to hash
 * @return The hash
 */
inline std::size_t chess::chessboard::hash::operator () ( const chessboard& cb ) const chess_validate_throw
{
    /* Set the hash to a random integer initially */
    bitboard hash_value { 0xc3efe6e59ff050d2 };

    /* Combine all bitboards into the hash */
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 6
    for ( int i = 0; i < 6; ++i ) hash_value ^= ( cb.bbs [ i ] [ 0 ] | cb.bbs [ i ] [ 1 ] ).bit_rotl ( i * 8 );
    hash_value ^= cb.bbs [ 6 ] [ 0 ].bit_rotl ( 48 ) ^ cb.bbs [ 6 ] [ 1 ].bit_rotl ( 56 );

    /* Incorporate aux info into hash */
    hash_value ^= bitboard { static_cast<unsigned> ( cb.aux_info.castling_rights ) };
    hash_value ^= bitboard { static_cast<unsigned> ( cb.aux_info.double_push_pos ) };

    /* Return the hash */
    return hash_value.get_value ();
}
inline std::size_t chess::chessboard::hash::operator () ( const game_state_t& cb ) const noexcept
{
    /* Set the hash to a random integer initially */
    bitboard hash_value { 0xcf4c987a6b0979 };

    /* Incorporate pc into hash */
    hash_value ^= bitboard { static_cast<unsigned> ( cast_penum ( cb.pc ) ) };

    /* Combine all bitboards into the hash */
    #pragma clang loop unroll ( full )
    #pragma GCC unroll 8
    for ( int i = 0; i < 8; ++i ) hash_value ^= cb.bbs [ i ].bit_rotl ( i * 8 );

    /* Incorporate aux info into hash */
    hash_value ^= bitboard { static_cast<unsigned> ( cb.aux_info.castling_rights ) };
    hash_value ^= bitboard { static_cast<unsigned> ( cb.aux_info.double_push_pos ) };

    /* Return the hash */
    return hash_value.get_value ();
}
inline std::size_t chess::chessboard::hash::operator () ( const move_t& mv ) const noexcept
{
    /* Return the hash of the positions */
    return mv.from ^ mv.to ^ cast_penum ( mv.pc ) ^ cast_penum ( mv.pt ) << 1 ^ cast_penum ( mv.capture_pt ) << 5 ^ cast_penum ( mv.promote_pt ) << 9;
}




/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_HPP_INCLUDED */