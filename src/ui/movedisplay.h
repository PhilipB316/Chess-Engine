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

#include "../movefinding/board.h"
#include "../movefinding/movefinder.h"

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

/**
 * @brief Determines the from square bitboard for a given move type and destination square.
 *
 * @param position The current position.
 * @param move_type The type of the move (e.g., PAWN, KNIGHT, etc.).
 * @param to_square The destination square index (0-63).
 * @param disambiguation The disambiguation string (e.g., file or rank).
 * @return A bitboard representing the from square for the move.
 */
ULL determine_from_square_bitboard(Position_t *position,
                                   MoveType_t move_type,
                                   uint8_t to_square,
                                   char *disambiguation);

/**
 * @brief Parses a move notation string and extracts the move type, destination square,
 * disambiguation, and whether it is a capture.
 *
 * @param move_notation The move notation string (e.g., "e4", "Nf3", "Bb5+", "O-O").
 * @param move_type Pointer to store the type of the move (e.g., PAWN, KNIGHT, etc.).
 * @param to_square Pointer to store the destination square index (0-63).
 * @param disambiguation Pointer to store any disambiguation characters (e.g., file or rank).
 * @param is_capture Pointer to store whether the move is a capture.
 * @param position Pointer to the current position to check for en passant and double push
 */
int parse_move_notation(char *move_notation,
                        MoveType_t *move_type,
                        uint8_t* to_square,
                        char *disambiguation,
                        bool *is_capture,
                        Position_t *position,
                        ULL* special_flags);

/**
 * @brief Determines the move notation for a move
 * 
 * @param old_position The position before the move
 * @param new_position The position after the move
 * @param move_notation Buffer to store the generated move notation
 */
void get_move_notation(Position_t* previous_position, Position_t* new_position, char* notation);

#endif // MOVEDISPLAY_H

