// search.c

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "search.h"
#include "evaluate.h"
#include "../movefinding/board.h"
#include "../movefinding/movefinder.h"

static ULL nodes_analysed = 0;
static uint64_t moves_generated = 0;

void sort_children(Position_t* postion, uint8_t depth);
int64_t negamax(Position_t* position, uint8_t depth, int64_t alpha, int64_t beta);

int64_t negamax_start(Position_t* position, Position_t* return_best_move, uint8_t depth)
{
    Position_t* best_move = NULL;
    int64_t alpha = -INT64_MAX;
    int64_t beta = INT64_MAX;
    int64_t best_eval = -INT64_MAX;
    // set half-move count for as current, rather than use the one from the position
    // as this will be dependent on the depth of the search
    set_half_move_count(position->half_move_count); 
    // depth move search one depth (generate single layer children)
    move_finder(position);
    sort_children(position, 4);

    for (uint16_t i = 0; i < position->num_children; i++)
    {
        printf("Nodes analysed: %llu, %d \r", nodes_analysed, 100 * i / position->num_children);
        fflush(stdout);
        Position_t* child = position->child_positions[i];
        int64_t eval = -negamax(child, depth - 1, -beta, -alpha);
        if (eval > best_eval)
        {
            best_eval = eval;
            best_move = child;
            if (best_eval > alpha)
            {
                alpha = best_eval;
            }
        }
    }

    // If no best move was found, return the original position
    if (best_move == NULL)
    {
        printf("No best move found!\n");
        *return_best_move = *position;
        free_children_memory(position);
        return 0;
    }

    *return_best_move = *best_move;
    free_children_memory(position);
    return best_eval;
}

int64_t negamax(Position_t* position, uint8_t depth, int64_t alpha, int64_t beta)
{
    if (depth == 0)
    {
        nodes_analysed++;
        return evaluate_position(position);
    }

    int64_t value = -INT64_MAX;
    move_finder(position);
    moves_generated++;
    for (uint16_t i = 0; i < position->num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        int64_t score = -negamax(child, depth - 1, -beta, -alpha);

        if (score > value)
        {
            value = score;
        }
        if (value > alpha)
        {
            alpha = value;
            if (alpha >= beta)
            {
                break; // Beta cutoff
            }
        }
    }
    free_children_memory(position);
    return value;
}

void sort_children(Position_t* position, uint8_t depth)
{
    int64_t evals[MAX_CHILDREN];
    uint16_t n = position->num_children;

    for (uint16_t i = 0; i < n; i++) {
        evals[i] = -negamax(position->child_positions[i], depth, -INT64_MAX, INT64_MAX);
    }

    // Simple insertion sort based on evals (descending order)
    for (uint8_t i = 1; i < n; i++) {
        int64_t key_eval = evals[i];
        Position_t* key_pos = position->child_positions[i];
        int j = i - 1;
        while (j >= 0 && evals[j] < key_eval) {
            evals[j + 1] = evals[j];
            position->child_positions[j + 1] = position->child_positions[j];
            j--;
        }
        evals[j + 1] = key_eval;
        position->child_positions[j + 1] = key_pos;
    }
    clear_children_count(position);
}

void find_best_move(Position_t* position, Position_t* return_best_move, uint8_t depth)
{
    clock_t start_time = clock();
    nodes_analysed = 0;
    moves_generated = 0;
    int64_t best_eval = negamax_start(position, return_best_move, depth);
    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("+-------------------------------------------+\n");
    printf("| %-24s | %14llu |\n", "Nodes analysed", nodes_analysed);
    printf("| %-24s | %14lu |\n", "Moves generated", moves_generated);
    printf("| %-24s | %14lu |\n", "New positions generated", get_num_new_positions());
    printf("| %-24s | %14ld |\n", "Best score", best_eval);
    printf("| %-24s | %14.2f |\n", "Time spent (seconds)", time_spent);
    printf("+-------------------------------------------+\n\n");
}

