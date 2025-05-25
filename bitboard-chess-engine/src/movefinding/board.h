/**
 * @file board_formatter.h
 * @brief Module for printing and handling chess boards.
 *
 * Provides functinality for
 * - printing bitboards
 * - printing positions
 * - converting FEN strings to positions
 * - printing children positions
 *
 * @author Philip Brand
 * @date 2025-04-01
 */

#ifndef BOARD
#define BOARD

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"

extern char *pretty_print_moves[64];

/**
 * @brief Prints the bitboard.
 *
 * @param bitboard The bitboard to be printed.
 */
void print_bitboard(uint64_t bitboard);

/**
 * @brief Prints the board position in ascii format.
 * 
 * Includes ranks and files labels for better readability.
 * 
 * @param position The position to be printed.
 */
void print_position(Position_t* position);

/**
 * @brief Converts a FEN string to a position, represented as bitboards.
 *
 * Includes castling rights, en passant square, piece values, and side to move.
 *
 * @param fen The FEN string representing the position.
 * @param position The position to be populated.
 */
void fen_to_board(char fen[], Position_t *position);

/**
 * @brief Prints the children positions of a given position.
 * @param position The position whose children are to be printed.
 */
void print_children_positions(Position_t *position);


/**
 * @brief Calculates the difference in piece values between the two players.
 * @param position The position to be evaluated.
 */
int16_t calculate_piece_value_diff(Position_t *position);

#endif // BOARD_FORMATTER_H
