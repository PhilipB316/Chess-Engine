// Piece.c

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "piece.h"
#include "../movefinding/board.h"

int PERSPECTIVE;

int16_t negamax(Position_t* position, uint8_t depth)
{
    if (depth == 0 || position->num_children == 0)
    {
        return position->piece_value_diff * PERSPECTIVE;
    }
    int16_t best_score = -10000;
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child_position = position->child_positions[i];
        int16_t score = negamax(child_position, depth - 1);
        if (score > best_score)
        {
            best_score = score;
        }
    }
    return best_score;
}

Position_t* find_best_move(Position_t* position, uint8_t depth, int colour)
{
    PERSPECTIVE = colour;
    Position_t* best_move;
    int16_t best_score = -1000;
    // Iterate through all possible moves
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child_position = position->child_positions[i];
        print_position(child_position);
        int16_t score = negamax(child_position, depth - 1);
        printf("Score: %d\n", score);
        if (score >= best_score)
        {
            best_score = score;
            best_move = child_position;
        }
    }
    return best_move;
}

