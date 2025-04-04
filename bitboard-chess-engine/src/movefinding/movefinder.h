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
    uint16_t en_passant;
    struct Position_t* parent_position;
    struct Position_t* child_positions[MAX_CHILDREN];
    uint8_t num_children;
} Position_t;

void move_finder_init(void);

void move_finder(Position_t *position);

#endif
