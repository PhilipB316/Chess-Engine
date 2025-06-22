/**
* @file movedisplay.h
* @brief Move display and notation generation
* @author Philip Brand
* @date 2025-06-21
*/

#ifndef MOVEDISPLAY_H
#define MOVEDISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "board.h"

/**
 * @brief Determines the move notation for a move from old_position to new_position.
 *
 * The move notation is stored in the provided move_notation buffer.
 *
 * @param old_position The position before the move.
 * @param new_position The position after the move.
 * @param move_notation Buffer to store the generated move notation.
 */
void determine_move_notation(Position_t *old_position, Position_t *new_position, char *move_notation);

void make_move_from_notation(char *move_notation, Position_t *source, Position_t *destination);

#endif // MOVEDISPLAY_H
