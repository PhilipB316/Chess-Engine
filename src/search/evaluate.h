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
#define CHECKMATE_VALUE 100000
#define INFINITY 1000000

// Values of each piece type for evaluation
#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 300
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 110000

// Weightings
#define CENTER_SQUARE_ATTACK_VALUE 20
#define BOX_SQUARE_ATTACK_VALUE 8
#define GENERAL_ATTACK_SQUARES_VALUE 2

// Evaluation thresholds
#define MID_GAME_MOVE_COUNT 6

// Masks
#define CENTER_FOUR_SQUARES 0x0000001818000000
#define BOX_SQUARES 0x00003C24243C0000
#define NOT_EDGE_RANKS 0x00FFFFFFFFFFFF00

/**
 * @brief Evaluates the position 
 * 
 * @param position The position to evaluate
 * @return The evaluation score of the position
 */
int64_t evaluate_position(Position_t* position);

/**
 * @brief Sets the half move count for the position.
 * 
 * @param count The half move count to set
 */
void set_half_move_count(uint16_t count);

#endif // EVALUATE_H
