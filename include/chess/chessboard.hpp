/*
 * Copyright (C) 2020 Louis Hobson <louis-hobson@hotmail.co.uk>. All Rights Reserved.
 *
 * Distributed under MIT licence as a part of the Chess C++ library.
 * For details, see: https://github.com/louishobson/Chess/blob/master/LICENSE
 *
 * include/chess/chessboard.hpp
 *
 * Inline implementation of include/chess/chessboard.h
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
inline constexpr bool chess::bool_color ( const pcolor pc ) noexcept { return static_cast<bool> ( pc ); }

/** @name  other_color
 * 
 * @brief  Take a piece color and give the other color
 * @param  pc: The piece color to switch. Undefined behavior if is no_piece.
 * @return The other color of piece
 */
inline constexpr chess::pcolor chess::other_color ( const pcolor pc ) noexcept { return static_cast<pcolor> ( !static_cast<bool> ( pc ) ); }



/* AB_STATE_T IMPLEMENTATION */

/** @name  chessboard constructor
 * 
 * @brief  Construct from a chessboard state
 * @param  cb: The chessboard to construct from
 * @param  pc: The player who's move it is next
 */
inline chess::chessboard::ab_state_t::ab_state_t ( const chessboard& cb, pcolor pc ) noexcept : bbs 
{
    cb.bb ( pcolor::white ),
    cb.bb ( pcolor::black ),
    cb.bb ( ptype::queen  ),
    cb.bb ( ptype::rook   ),
    cb.bb ( ptype::bishop ),
    cb.bb ( ptype::knight ),
    cb.bb ( ptype::pawn   ),
    cb.bb ( ptype::king   )
}, pc_and_check_info { static_cast<int> ( pc ) | cb.castling_rights << 1 } {}



/* HASHING FUNCTION IMPLEMENTATION */



/** @name  operator ()
 * 
 * @brief  Creates a hash for a chessboard
 * @param  cb: The chessboard or alpha-beta state to hash
 * @param  mv: The move to hash
 * @return The hash
 */
inline std::size_t chess::chessboard::hash::operator () ( const chessboard& cb ) const noexcept
{
    /* Simply return the occupied positions */
    return cb.bb ().get_value ();
}
inline std::size_t chess::chessboard::hash::operator () ( const ab_state_t& cb ) const noexcept
{
    /* Simply return the occupied positions */
    return ( cb.bbs [ 0 ] | cb.bbs [ 1 ] ).get_value ();
}
inline std::size_t chess::chessboard::hash::operator () ( const move_t& mv ) const noexcept
{
    /* Return union of the positions */
    return ( mv.from_bb | mv.to_bb ).get_value ();
}



/* CHESSBOARD CONSTRUCTORS AND OPERATORS */



/** @name  copy constructor
 * 
 * @brief  Copy constructs the chess board
 */
inline chess::chessboard::chessboard ( const chessboard& other ) noexcept
    /* Initialize values */
    : color_bbs       { other.color_bbs }
    , type_bbs        { other.type_bbs }
    , castling_rights { other.castling_rights }

    /* Don't create ab_working, since it will be created if a search occured */
{}

/** @name  copy assignment operator
 * 
 * @brief  Copy assigns the chess board
 */
inline chess::chessboard& chess::chessboard::operator= ( const chessboard& other ) noexcept
{
    /* Copy over values */
    color_bbs       = other.color_bbs;
    type_bbs        = other.type_bbs;
    castling_rights = other.castling_rights;

    /* Don't create ab_working, since it will be created if a search occured */

    /* Return this object */
    return * this;
}

/** @name  destructor
 * 
 * @brief  Destructs the chessboard
 */
inline chess::chessboard::~chessboard () noexcept
{
    /* Destroy the working values, if not already */
    if ( ab_working ) { delete ab_working; ab_working = nullptr; }
}

/** @name  operator==
 * 
 * @brief  Compares if two chessboards are equal
 */
inline bool chess::chessboard::operator== ( const chessboard& other ) const noexcept
{
    /* Compare and return */
    return ( ( color_bbs == other.color_bbs ) && ( type_bbs == other.type_bbs ) && ( castling_rights == other.castling_rights ) );
}



/* FIND COLOR AND TYPE */



/** @name  find_color
 *
 * @brief  Determines the color of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pos: Board position
 * @return One of pcolor
 */
inline chess::pcolor chess::chessboard::find_color ( const unsigned pos ) const noexcept
{
    const bitboard mask = singleton_bitboard ( pos );
    if ( bb ( pcolor::white ) & mask ) return pcolor::white; else
    if ( bb ( pcolor::black ) & mask ) return pcolor::black; else
    return pcolor::no_piece;
}

/** @name  find_type
 * 
 * @brief  Determines the type of piece at a board position.
 *         Note: an out of range position leads to undefined behavior.
 * @param  pc:  The known piece color
 * @param  pos: Board position
 * @param  pos_bb: Singleton bitboard
 * @return One of ptype
 */
inline chess::ptype chess::chessboard::find_type ( const pcolor pc, const unsigned pos ) const noexcept
{
    /* Get a singleton bitboard and recall */
    return find_type ( pc, singleton_bitboard ( pos ) );
}
inline chess::ptype chess::chessboard::find_type ( const pcolor pc, const bitboard pos_bb ) const noexcept
{
    /* First detect if there is a piece, then find what type it is */
    if ( !bb ( pc ).contains ( pos_bb ) ) return ptype::no_piece;
    if ( bb ( pc, ptype::pawn   ).contains ( pos_bb ) ) return ptype::pawn;   else
    if ( bb ( pc, ptype::knight ).contains ( pos_bb ) ) return ptype::knight; else
    if ( bb ( pc, ptype::bishop ).contains ( pos_bb ) ) return ptype::bishop; else
    if ( bb ( pc, ptype::rook   ).contains ( pos_bb ) ) return ptype::rook;   else
    if ( bb ( pc, ptype::queen  ).contains ( pos_bb ) ) return ptype::queen;  else
    if ( bb ( pc, ptype::king   ).contains ( pos_bb ) ) return ptype::king;   else
    return ptype::no_piece;
}



/* ATTACK LOOKUPS */



/** @name  any_attack_lookup
 * 
 * @brief  lookup an attack set based on a type
 * @param  pt: The type of the piece
 * @param  pos: The position of the piece
 * @return A bitboard for the attack set
 */
inline constexpr chess::bitboard chess::chessboard::any_attack_lookup ( ptype pt, unsigned pos ) const noexcept
{
    /* Switch based on pt */
    switch ( pt )
    {
    case ( ptype::queen  ): return bitboard::queen_attack_lookup    ( pos );
    case ( ptype::rook   ): return bitboard::straight_attack_lookup ( pos );
    case ( ptype::bishop ): return bitboard::diagonal_attack_lookup ( pos );
    case ( ptype::knight ): return bitboard::knight_attack_lookup   ( pos );
    //case ( ptype::pawn   ): return bitboard {};
    case ( ptype::king   ): return bitboard::king_attack_lookup     ( pos ); 
    default: return bitboard {};
    }
}



/* HEADER GUARD */
#endif /* #ifndef CHESS_CHESSBOARD_HPP_INCLUDED */