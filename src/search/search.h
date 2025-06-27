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
 * @param depth The maximum depth to search.
 * @return Whether the position is checkmate or not.
 */
uint8_t find_best_move(Position_t* position, Position_t* return_best_move, uint8_t depth);



/**
 * @brief Prints the statistics of the search.
 */
void print_stats(void);

#endif // SEARCH_H
