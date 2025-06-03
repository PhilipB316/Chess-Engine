// search.c

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "search.h"
#include "evaluate.h"
#include "../movefinding/board.h"
#include "../movefinding/movefinder.h"

static ULL nodes_analysed = 0;

int16_t negamax(Position_t* position, uint8_t depth)
{
    if (depth == 0)
    {
        nodes_analysed++;
        return evaluate_position(position);
    }
    int16_t value = -10000;
    depth_move_finder(position, 1);
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        int16_t score = -negamax(child, depth - 1);
        if (score > value)
        {
            value = score;
        }
    }
    free_children_memory(position);
    return value;
}

void find_best_move(Position_t* position, Position_t* return_best_move, uint8_t depth)
{
    Position_t* best_move = NULL;
    int16_t best_eval = -10000;
    depth_move_finder(position, 1);

    printf("Num nodes: %d\n", position->num_children);

    for (uint8_t i = 0; i < position->num_children; i++)
    {
        printf("Nodes analysed: %llu\r", nodes_analysed);
        fflush(stdout);
        Position_t* child = position->child_positions[i];
        int16_t eval = -negamax(child, depth - 1);
        if (eval > best_eval)
        {
            best_eval = eval;
            best_move = child;
        }
    }
    printf("Nodes analysed: %llu\n", nodes_analysed);
    *return_best_move = *best_move;
    free_children_memory(position);
}

