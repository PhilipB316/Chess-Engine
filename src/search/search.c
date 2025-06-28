// search.c

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

#include "search.h"
#include "evaluate.h"
#include "../movefinding/board.h"
#include "../movefinding/movefinder.h"

static ULL nodes_analysed = 0;
static uint64_t moves_generated = 0;
static int64_t best_eval = 0;
static uint8_t searched_depth = 0;
static clock_t start_time = 0;
static clock_t global_max_time = 0;
static bool time_up = false;
static double time_spent = 0.0;

void sort_children(Position_t* postion);
int64_t negamax(Position_t* position, uint8_t depth, int64_t alpha, int64_t beta);
int64_t negamax_start(Position_t* position, Position_t* return_best_move, uint8_t depth);

static inline bool time_is_up(void)
{
    time_up = (clock() - start_time) >= global_max_time;
    return time_up;
}

void find_best_move(Position_t* position, 
                       Position_t* return_best_move, 
                       uint8_t max_depth,
                       clock_t max_time)
{
    start_time = clock();
    global_max_time = max_time * CLOCKS_PER_SEC; // Convert seconds to clock ticks
    nodes_analysed = 0;
    moves_generated = 0;
    searched_depth = 1;

    move_finder(position);

    while (!time_is_up() && searched_depth < max_depth)
    {
        printf("Running negamax with depth %d\r", searched_depth);
        fflush(stdout);
        sort_children(position);
        best_eval = negamax_start(position, return_best_move, searched_depth);
        searched_depth++;
    }

    free_children_memory(position);

    clock_t end_time = clock();
    time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
}

int64_t negamax_start(Position_t* position, 
                      Position_t* return_best_move,
                      uint8_t depth)
{
    Position_t* best_move = NULL;
    int64_t alpha = -INT64_MAX;
    int64_t beta = INT64_MAX;
    int64_t best_eval = -INT64_MAX;

    for (uint16_t i = 0; i < position->num_children; i++)
    {
        if (time_is_up()) { return 0; }

        printf("Nodes analysed: %llu, %d \r", nodes_analysed, 100 * i / position->num_children);
        fflush(stdout);
        Position_t* child = position->child_positions[i];
        int64_t eval = -negamax(child, depth - 1, -beta, -alpha);
        if (time_up) { return 0; /* Time ran out in deeper search */ }

        child->evaluation = eval;
        if (eval > best_eval) {
            best_eval = eval;
            best_move = child;
            if (best_eval > alpha) { alpha = best_eval; }
        }
    }
    clear_grandchildren_count(position);

    // If no best move was found, return the original position
    if (best_move == NULL) { *return_best_move = *position; return 0; }
    if (return_best_move == NULL) { return best_eval; /* No best move to return */ }

    *return_best_move = *best_move;
    return best_eval;
}

int64_t negamax(Position_t* position, uint8_t depth, int64_t alpha, int64_t beta)
{
    if (depth == 0) {
        nodes_analysed++;
        return evaluate_position(position);
    }

    int64_t value = -INT64_MAX;
    move_finder(position);
    moves_generated++;
    for (uint16_t i = 0; i < position->num_children; i++)
    {
        if (time_is_up()) {
            free_children_memory(position); // must free children before returning
            return 0;
        }
        Position_t* child = position->child_positions[i];
        int64_t score = -negamax(child, depth - 1, -beta, -alpha);

        if (score > value) { value = score; }
        if (value > alpha) {
            alpha = value;
            if (alpha >= beta) { break; /* Beta cutoff */ }
        }
    }
    free_children_memory(position); // must free children before returning
    return value;
}

void sort_children(Position_t* position)
{
    uint16_t n = position->num_children;

    // Simple insertion sort based on evals (descending order)
    for (uint8_t i = 1; i < n; i++) {
        int64_t key_eval = position->child_positions[i]->evaluation;
        Position_t* key_pos = position->child_positions[i];
        int j = i - 1;
        while (j >= 0 && position->child_positions[j]->evaluation < key_eval) {
            position->child_positions[j+1]->evaluation = position->child_positions[j]->evaluation;
            position->child_positions[j + 1] = position->child_positions[j];
            j--;
        }
        position->child_positions[j+1]->evaluation = key_eval;
        position->child_positions[j + 1] = key_pos;
    }
    clear_grandchildren_count(position);
}

void print_stats(void)
{
    printf("+-------------------------------------------+\n");
    printf("| %-24s | %14lu |\n", "New positions generated", get_num_new_positions());
    printf("| %-24s | %14llu |\n", "Nodes analysed", nodes_analysed);
    printf("| %-24s | %14lu |\n", "Moves generated", moves_generated);
    printf("| %-24s | %14.2f |\n", "Time spent (seconds)", time_spent);
    printf("| %-24s | %14u |\n", "Depth", searched_depth - 1); // depth
    // printf("| %-24s | %14ld |\n", "Best score", best_eval);
    printf("+-------------------------------------------+\n\n");
}
