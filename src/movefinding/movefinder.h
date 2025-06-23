/**
 * @file movefinder.h
 * @brief movefinding
 * @author Philip Brand
 * @date 2024-12-06
 */

#ifndef MOVEFINDER_H
#define MOVEFINDER_H

#include <stdint.h>
#include <stdbool.h>

#include "board.h"

/**
 * @brief Enum for the different piece types moves.
 */
typedef enum 
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    DOUBLE_PUSH,
    PROMOTE_QUEEN,
    PROMOTE_ROOK,
    PROMOTE_BISHOP,
    PROMOTE_KNIGHT,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
    EN_PASSANT_CAPTURE,
} MoveType_t;

/**
 * @brief Initialises lookup tables for move finding.
 */
void move_finder_init(void);

/**
 * @brief Generates all possible moves for the current position.
 *
 * @param position The position to generates move for.
 */
void move_finder(Position_t *position);

/**
 * @brief Free memory from allocated position and children.
 * DOES FREE THE POSITION ITSELF.
 *
 * @param position The position to free memory for.
 */
void free_children_memory(Position_t *position);

/**
    * @brief Free all memory allocated to position children.
    * DOES NOT FREE THE POSITION ITSELF.
    * 
    * @param position The position to free the children of
*/
void free_position_memory(Position_t *position);

/**
 * @brief Free all memory allocated to specific depth.
 * DOES FREE THE POSITION ITSELF.
 *
 * @param position The position to free memory for.
 * @param depth The depth of the search.
 */
void free_depth_memory(Position_t* position, uint8_t depth);

/**
 * @brief Finds all possible moves for the current position to the specified depth.
 *
 * @param position The position to generate moves for.
 * @param depth The depth of the search.
 */
void depth_move_finder(Position_t* position, uint8_t depth);

/**
* @brief Gets the number of new positions generated during move finding.
* @return The number of new positions generated.
*/
uint64_t get_num_new_positions(void);

/**
* @brief Clears the children count for the given position.
* This resets the number of children to 0 for all child positions.
*
* @param position The position to clear the children count for.
*/
void clear_children_count(Position_t *position);


// ---------------------- THE FOLLOWING FUNCTIONS ARE FOR MOVE DISPLAY ----------------------

ULL find_knight_moves(Position_t *position, uint8_t knight_square);
ULL find_bishop_moves(Position_t *position, uint8_t bishop_square);
ULL find_rook_moves(Position_t *position, uint8_t rook_square);
ULL find_queen_moves(Position_t *position, uint8_t queen_square);
ULL find_king_moves(Position_t *position, uint8_t king_square);
ULL find_pawn_moves(Position_t* position, uint8_t pawn_square);
void make_notation_move(Position_t *old_position,
                        Position_t *new_position,
                        MoveType_t piece,
                        ULL to_square_bitboard, 
                        ULL from_square_bitboard, 
                        ULL special_flags);


#endif // MOVEFINDER_H
