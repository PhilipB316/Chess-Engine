/**
 * @file search.h
 * @brief negamax search algorithm
 * @author Philip Brand
 * @date 2025-04-12
 */

#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>
#include <stdbool.h>

#include "../movefinding/movefinder.h"

/**
 * @brief Negamax search algorithm for a given position and depth.
 * 
 * This function recursively searches for the best move by evaluating the position
 * Colour is intrinsically determined
 * 
 * @param position The current position to evaluate.
 * @param depth The maximum depth to search.
 * @return The resulting position from the best move
 */
Position_t* find_best_move(Position_t* position, uint8_t depth);

#endif // SEARCH_H
