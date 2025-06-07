// search.c

#include <stdint.h>
#include <stdio.h>

#include "search.h"
#include "evaluate.h"
#include "../movefinding/board.h"
#include "../movefinding/movefinder.h"

static ULL nodes_analysed = 0;
static uint64_t moves_generated = 0;

int64_t negamax(Position_t* position, uint8_t depth)
{
    if (depth == 0)
    {
        nodes_analysed++;
        return evaluate_position(position);
    }

    int64_t value = -INFINITY;
    move_finder(position);
    moves_generated++;
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        int64_t score = -negamax(child, depth - 1);
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
    int64_t best_eval = -INFINITY;
    set_half_move_count(position->half_move_count);

    move_finder(position);
    printf("Immediate children: %d\n", position->num_children);

    for (uint8_t i = 0; i < position->num_children; i++)
    {
        printf("Nodes analysed: %llu, %d \r", nodes_analysed, 100 * i / position->num_children);
        fflush(stdout);
        Position_t* child = position->child_positions[i];
        int64_t eval = -negamax(child, depth - 1);
        if (eval > best_eval)
        {
            best_eval = eval;
            best_move = child;
        }
    }

    printf("Nodes analysed: %llu       \n", nodes_analysed);
    printf("Moves generated: %lu\n", moves_generated);
    printf("Best score: %ld\n", best_eval);
    if (best_move == NULL)
    {
        printf("No best move found!\n");
        *return_best_move = *position; // Return the original position if no best move is found
        free_children_memory(position);
        return;
    }
    *return_best_move = *best_move;
    free_children_memory(position);
}

