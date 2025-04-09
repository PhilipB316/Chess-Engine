/**
 * @file movefinder.h
 * @brief movefinding
 * @author Philip Brand
 * @date 2024-12-06
 */

#ifndef MOVEFINDER_H
#define MOVEFINDER_H

#include <stdint.h>
#include <stdbool.h>

#define ULL unsigned long long
#define MAX_CHILDREN 100

typedef enum 
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    DOUBLE_PUSH,
    PROMOTE_QUEEN,
    PROMOTE_ROOK,
    PROMOTE_BISHOP,
    PROMOTE_KNIGHT,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
    EN_PASSANT_CAPTURE,
} PieceType;

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

typedef struct Position_t
{
    PiecesOneColour_t white_pieces;
    PiecesOneColour_t black_pieces;
    ULL all_pieces;
    bool white_to_move;
    ULL en_passant_bitboard;
    struct Position_t* parent_position;
    struct Position_t* child_positions[MAX_CHILDREN];
    uint8_t num_children;
} Position_t;

void move_finder_init(void);

void move_finder(Position_t *position);

/**
    * @brief Free all memory alloaced for the childern of the position
    * 
    * @param position The position to free the children of
*/
void free_position_memory(Position_t *position);


#endif
