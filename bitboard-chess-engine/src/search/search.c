// search.c

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "search.h"
#include "evaluate.h"
#include "../movefinding/board.h"

static ULL nodes_analysed = 0;

int16_t negamax(Position_t* position, uint8_t depth)
{
    if (depth == 0 || position->num_children == 0)
    {
        nodes_analysed++;
        return evaluate_position(position);
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

    printf("Num nodes: %d\n", position->num_children);

    for (uint8_t i = 0; i < position->num_children; i++)
    {
        printf("Nodes analysed: %llu\r", nodes_analysed);
        fflush(stdout);
        depth_move_finder(position->child_positions[i], depth - 1);
        Position_t* child = position->child_positions[i];
        int16_t eval = -negamax(child, depth - 1);
        if (eval > best_eval)
        {
            best_eval = eval;
            best_move = child;
        }
        free_position_memory(child); // Free memory for the child position
    }
    printf("Nodes analysed: %llu\n", nodes_analysed);
    return best_move;
}

