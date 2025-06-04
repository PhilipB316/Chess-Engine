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

#endif
