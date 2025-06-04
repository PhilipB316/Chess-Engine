/**
* @file evaluate.h
* @brief Evaluation of chess position for search algorithm
* @author Philip Brand
* @date 2025-06-02
*/

#ifndef EVALUATE_H
#define EVALUATE_H

#include <stdint.h>

#include "../movefinding/board.h"

// Piece colour signs
#define WHITE_PIECE_COLOUR 1
#define BLACK_PIECE_COLOUR -1

#define CHECKMATE_VALUE 1000

// Values of each piece type for evaluation
#define PAWN_VALUE 1
#define KNIGHT_VALUE 3
#define BISHOP_VALUE 3
#define ROOK_VALUE 5
#define QUEEN_VALUE 9
#define KING_VALUE 1100

/**
 * @brief Evaluates the position 
 * 
 * @param position The position to evaluate
 * @return The evaluation score of the position
 */
int32_t evaluate_position(const Position_t* position);

#endif // EVALUATE_H
