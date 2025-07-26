/**
 * @file transposition_tables.h
 * @brief transposition table
 * @author Philip Brand
 * @date 2025-25-07
 */

#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <stdint.h>

#include "board.h"

extern ULL zobrist_key_table[2][6][64];
extern ULL zobrist_black_to_move;
extern ULL zobrist_en_passant[65];
extern ULL zobrist_castling[2][2];

/**
 * @brief Initialises the Zobrist key table with random values.
 */
void zobrist_key_init(void);

/**
 * @brief Generates a Zobrist hash for the given position.
 *
 * The Zobrist hash is a unique identifier for the position, taking into account
 * the pieces on the board, their positions, castling rights, en passant square,
 * and whether it is white's turn to move.
 *
 * @param position The position to generate the Zobrist hash for.
 * @return The generated Zobrist hash as an unsigned long long integer.
 */
ULL generate_zobrist_hash(Position_t *position);

#endif // TRANSPOSITION_TABLE_H

