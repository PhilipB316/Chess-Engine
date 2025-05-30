// Piece.c

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "piece.h"

int16_t negamax(Position_t* position, uint8_t depth)
{
    if (depth == 0 || position->num_children == 0)
    {
        return position->piece_value_diff * (position->white_to_move ? 1 : -1); // Return piece value difference
    }
    int16_t value = -10000;
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        int16_t score = -negamax(child, depth - 1);
        if (score > value)
        {
            value = score;
        }
    }
    return value;
}

Position_t* find_best_move(Position_t* position, uint8_t depth)
{
    Position_t* best_move = NULL;
    int16_t best_eval = -10000;

    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        int16_t eval = -negamax(child, depth - 1);
        if (eval > best_eval)
        {
            best_eval = eval;
            best_move = child;
        }
    }
    return best_move;
}

