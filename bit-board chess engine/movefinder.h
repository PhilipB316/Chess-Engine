/**
 * @file movefinder.h
 * @brief movefinding, bitboard printing and move printing functions
 * @author Philip Brand
 * @date 2024-12-06
 *
 * NOTE: bitboard and fen conversion functions not to be implemented here later.
 */

#ifndef MOVEFINDER_H
#define MOVEFINDER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define ULL unsigned long long

typedef struct
{
    ULL pawns;
    ULL knights;
    ULL bishops;
    ULL rooks;
    ULL queens;
    ULL kings;
    ULL all_pieces;
    bool castle_kingside;
    bool castle_queenside;
} PiecesOneColour_t;

typedef struct
{
    PiecesOneColour_t white_pieces;
    PiecesOneColour_t black_pieces;
    ULL all_pieces;
    bool white_to_move;
    uint16_t en_passant;
} Position_t;

typedef enum
{
    NONE = 0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} Piece_t;

typedef enum
{
    NORMAL = 0,
    PROMOTION,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
    DOUBLE_PAWN_PUSH,
    EN_PASSANT_CAPTURE
} MoveType_t;


void move_finder(Position_t *position_list,
                 size_t *num_positions,
                 Position_t *const position,
                 PiecesOneColour_t *active_pieces,
                 PiecesOneColour_t *opponent_pieces);

#endif