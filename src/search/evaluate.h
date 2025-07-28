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

// Values of each piece type for evaluation
#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 300
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 110000

// Weightings
#define CENTER_SQUARE_ATTACK_VALUE 18
#define PAWN_CENTER_ATTACK_VALUE_OFFSET 50
#define KNIGHT_CENTER_ATTACK_VALUE_OFFSET 40
#define BOX_SQUARE_ATTACK_VALUE 1
#define GENERAL_ATTACK_SQUARES_VALUE 1

// Evaluation thresholds
#define MID_GAME_MOVE_COUNT 15

// Masks
#define CENTER_FOUR_SQUARES 0x0000001818000000
#define BOX_SQUARES 0x00003C24243C0000
#define NOT_EDGE_RANKS 0x00FFFFFFFFFFFF00

typedef enum {
    BORING,
    CHECK,
    CHECKMATE,
    STALEMATE
} KingStatus_t;

/**
 * @brief Evaluates the position 
 * 
 * @param position The position to evaluate
 * @return The evaluation score of the position
 */
int32_t evaluate_position(Position_t* position);

/**
 * @brief Determines if the position is check.
 *
 * @param position The position to check
 * @return true if the position is check, false otherwise
 */
bool is_check(Position_t* position);

/**
 * @brief Determines the status of the king in the position.
 *
 * @param position The position to check
 * @return The status of the king (CHECK, CHECKMATE, STALEMATE, or BORING)
 */
KingStatus_t determine_king_status(Position_t* position);

#endif // EVALUATE_H
