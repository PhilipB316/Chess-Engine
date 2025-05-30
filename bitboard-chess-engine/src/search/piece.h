/**
 * @file piece.h
 * @brief negamax search algorithm
 * @author Philip Brand
 * @date 2025-04-12
 */

#ifndef PIECE_H
#define PIECE_H

#include <stdint.h>
#include <stdbool.h>

#include "../movefinding/movefinder.h"

#define WHITE_PIECE_COLOUR 1
#define BLACK_PIECE_COLOUR -1

#define PAWN_VALUE 1
#define KNIGHT_VALUE 3
#define BISHOP_VALUE 3
#define ROOK_VALUE 5
#define QUEEN_VALUE 9
#define KING_VALUE 500

/**
 * @brief Negamax search algorithm for a given position and depth.
 * 
 * This function recursively searches for the best move by evaluating the position
 * and its children positions up to a specified depth.
 * 
 * @param position The current position to evaluate.
 * @param depth The maximum depth to search.
 * @param colour The colour of the player to move (1 for white, -1 for black).
 * @return The resulting position from the best move
 */
Position_t* find_best_move_colour(Position_t* position, uint8_t depth, int colour);

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
Position_t* find_best_move_simple(Position_t* position, uint8_t depth);

#endif // PIECE_H

