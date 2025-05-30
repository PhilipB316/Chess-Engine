// Piece.c

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "piece.h"
#include "../movefinding/board.h"

int PERSPECTIVE;

int16_t negamax_colour(Position_t* position, uint8_t depth, int colour)
{
    if (depth == 0 || position->num_children == 0)
    {
        return position->piece_value_diff * colour;// * PERSPECTIVE;
    }
    int16_t best_score = -10000;
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child_position = position->child_positions[i];
        int16_t score = - negamax_colour(child_position, depth - 1, -colour);
        if (score > best_score)
        {
            best_score = score;
        }
    }
    return best_score;
}

Position_t* find_best_move_colour(Position_t* position, uint8_t depth, int colour)
{
    PERSPECTIVE = colour;
    Position_t* best_move;
    int16_t best_score = -10000;
    // Iterate through all possible moves
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child_position = position->child_positions[i];
        // print_position(child_position);
        int16_t score = - negamax_colour(child_position, depth - 1, -colour);
        printf("Score: %d\n", score);
        if (score > best_score)
        {
            best_score = score;
            best_move = child_position;
        }
    }
    return best_move;
}

int16_t negamax_simple(Position_t* position, uint8_t depth)
{
    if (depth == 0 || position->num_children == 0)
    {
        return position->piece_value_diff * (position->white_to_move ? 1 : -1); // Return piece value difference
    }
    int16_t value = -10000;
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        int16_t score = -negamax_simple(child, depth - 1);
        if (score > value)
        {
            value = score;
        }
    }
    return value;
}

Position_t* find_best_move_simple(Position_t* position, uint8_t depth)
{
    Position_t* best_move = NULL;
    int16_t best_eval = -10000;

    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        int16_t eval = -negamax_simple(child, depth - 1);
        if (eval > best_eval)
        {
            best_eval = eval;
            best_move = child;
        }
    }
    return best_move;
}

