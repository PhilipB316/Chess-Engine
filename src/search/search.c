// search.c

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "search.h"
#include "evaluate.h"
#include "hash_tables.h"
#include "../movefinding/board.h"
#include "../movefinding/movefinder.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static ULL nodes_analysed = 0;
static uint64_t moves_generated = 0;
static int32_t best_eval = 0;
static uint8_t searched_depth = 0;

static clock_t start_time = 0;
static clock_t global_max_time = 0;
static bool time_up = false;
static double time_spent = 0.0;

void sort_children(Position_t* postion);
int32_t negamax(Position_t* position, uint8_t depth, int32_t alpha, int32_t beta);
int32_t negamax_start(Position_t* position, Position_t* return_best_move, uint8_t depth);

static inline bool time_is_up(void)
{
    time_up = (clock() - start_time) >= global_max_time;
    return time_up;
}

void find_best_move(Position_t* position, 
                    Position_t* return_best_move, 
                    uint8_t max_depth,
                    long long max_time) // max time in milliseconds
{
    start_time = clock();
    global_max_time = max_time * CLOCKS_PER_SEC / 1000; // Convert milliseconds to clock ticks
    nodes_analysed = 0;
    moves_generated = 0;
    searched_depth = 1;

    move_finder(position);

    while (!time_is_up() && searched_depth < max_depth)
    {
        sort_children(position);
        best_eval = negamax_start(position, return_best_move, searched_depth);
        searched_depth++;
    }

    // if no best move is found then mate in one
    // if mate in one then the best move is whatever is possible
    // at a depth of 1
    if (position->zobrist_key == return_best_move->zobrist_key) {
        negamax_start(position, return_best_move, 1);
    }

    free_children_memory(position);

    clock_t end_time = clock();
    time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
}

int32_t negamax_start(Position_t* position, 
                      Position_t* return_best_move,
                      uint8_t depth)
{
    // --- alpha-beta negamax search initialisation ---
    Position_t* best_move = NULL;
    int32_t alpha = -INT32_MAX;
    int32_t beta = INT32_MAX;
    int32_t best_eval = -INT32_MAX;

    // --- main negamax loop over child positions ---
    for (uint16_t i = 0; i < position->num_children; i++)
    {
        if (time_is_up()) { return 0; }

        Position_t* child = position->child_positions[i];
        // insert into past move list for repetition detection
        insert_past_move_entry(child);
        int32_t eval = -negamax(child, depth - 1, -beta, -alpha);
        // remove from past move list
        clear_past_move_entry(child);
        if (time_up) { return 0; /* Time ran out in deeper search */ }

        child->evaluation = eval;
        if (eval > best_eval) {
            best_eval = eval;
            best_move = child;
            if (best_eval > alpha) { alpha = best_eval; }
        }
    }
    clear_grandchildren_count(position);

    // --- return best move and evaluation ---
    if (best_move == NULL) { *return_best_move = *position; return 0; }
    /* No best move to return, don't populate return_best_move */
    if (return_best_move == NULL) { return best_eval; }
    *return_best_move = *best_move;
    return best_eval;
}

int32_t negamax(Position_t* position, uint8_t depth, int32_t alpha, int32_t beta)
{
    // --- base case ---
    if (depth == 0) {
        nodes_analysed++;
        return evaluate_position(position);
    }
    // --- repetition detection ---
    if (is_threefold_repetition(position)) {
        nodes_analysed++;
        return 0; // Draw by repetition
    }

    // --- transposition table lookup ---
    ULL key = position->zobrist_key;
    TranspositionEntry_t* entry = &transposition_table[key & TT_MASK];
    bool tt_move_found = false;
    if (entry->zobrist_key == key) {
        tt_move_found = true; 
        // NOTE: not sure this is actually working correctly

        if (entry->search_depth == depth) {
            nodes_analysed++;
            if (entry->node_type == EXACT) {
                return entry->position_evaluation; /* Exact match */
            } else if (entry->node_type == LOWER_BOUND) {
                alpha = entry->position_evaluation; // Lower bound
            } else if (entry->node_type == UPPER_BOUND) {
                beta = entry->position_evaluation; // Upper bound
            }
        }
        // if (alpha >= beta) { return entry->position_evaluation; /* Cutoff */ }
    }

    int32_t value = -INT32_MAX;
    move_finder(position);
    moves_generated++;

    // --- transposition table move ordering optimisation ---
    if (tt_move_found) {
        for (uint16_t i = 0; i < position->num_children; i++) {
            if (position->child_positions[i]->zobrist_key == entry->best_move_zobrist_key) {
                if (i != 0) {
                    Position_t* tmp = position->child_positions[0];
                    position->child_positions[0] = position->child_positions[i];
                    position->child_positions[i] = tmp;
                }
                break;
            }
        }
    }

    // --- main negamax loop ---
    int best_child_index = -1;
    for (uint16_t i = 0; i < position->num_children; i++)
    {
        if (time_is_up()) {
            free_children_memory(position); // must free children before returning
            return 0;
        }
        Position_t* child = position->child_positions[i];
        insert_past_move_entry(child);
        int32_t score = -negamax(child, depth - 1, -beta, -alpha);
        clear_past_move_entry(child);

        // --- track best score and move index ---
        if (score > value) {
            value = score;
            best_child_index = i;
        }
        // --- alpha-beta pruning ---
        if (value > alpha) {
            alpha = value;
            if (alpha >= beta) { break; /* Beta cutoff */ }
        }
    }

    // --- update transposition table with best move and score ---
    if (best_child_index >= 0) {
        entry->zobrist_key = key;
        entry->position_evaluation = value;
        entry->half_move_count = position->half_move_count;
        entry->search_depth = depth;
        if (value <= alpha) { entry->node_type = UPPER_BOUND; /* Upper bound */ } 
        else if (value >= beta) { entry->node_type = LOWER_BOUND; /* Lower bound */ } 
        else { entry->node_type = EXACT; /* Exact match */ }
        entry->best_move_zobrist_key = position->child_positions[best_child_index]->zobrist_key;
    }

    // --- cleanup and return ---
    free_children_memory(position); // must free children before returning
    return value;
}

void sort_children(Position_t* position)
{
    uint16_t n = position->num_children;

    // Simple insertion sort based on evals (descending order)
    for (uint8_t i = 1; i < n; i++) {
        int32_t key_eval = position->child_positions[i]->evaluation;
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
    printf("| %-24s | %14.4f |\n", "Time spent (seconds)", time_spent);
    printf("| %-24s | %14u |\n", "Depth", searched_depth - 1); // depth
    printf("+-------------------------------------------+\n\n");
}

