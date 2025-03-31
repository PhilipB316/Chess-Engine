/**
 * @file board_formatter.h
 * @brief This file contains functions for printing the board 
 * and converting FEN to bitboards.
 * @author Philip Brand
 * @date 2025-04-01
 */

#ifndef BOARD_FORMATTER_H
#define BOARD_FORMATTER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"

extern char *pretty_print_moves[64];

void print_position(Position_t const position);

/**
 * @brief Converts a FEN string to a position, represented as bitboards.
 *
 * Includes castling rights, en passant square, and side to move.
 *
 * @param fen The FEN string representing the position.
 * @return The board position represented as a Position_t structure.
 */
Position_t fen_to_board(char fen[]);

/**
 * @brief Prints the bitboard.
 *
 * @param bitboard The bitboard to be printed.
 */
void print_bitboard(uint64_t bitboard);

#endif // BOARD_FORMATTER_H
