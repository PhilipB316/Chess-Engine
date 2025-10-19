/**
 * @file search.h
 * @brief negamax search algorithm
 * @author Philip Brand
 * @date 2025-04-12
 */

#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>

#include "../movefinding/board.h"

/**
 * @brief Negamax search algorithm for a given position and depth.
 * 
 * This function recursively searches for the best move by evaluating the position
 * Colour is intrinsically determined
 * 
 * @param position The current position to evaluate.
 * @param best_move The position to store the best move found.
 * @param max_depth The maximum depth to search.
 * @param max_time The maximum time allowed for the search.
 */
void find_best_move(Position_t* position, 
                       Position_t* return_best_move, 
                       uint8_t max_depth,
                       long long max_time);

/**
 * @brief Passes the past move list pointer to the search module.
 *
 * @param past_move_list Pointer to the head of the past move list.
 */
void pass_movelist_pointer_to_search(PastMoveListEntry_t* past_move_list);

/**
 * @brief Prints the statistics of the search.
 */
void print_stats(void);

#endif // SEARCH_H
