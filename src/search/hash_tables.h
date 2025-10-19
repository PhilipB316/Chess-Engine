/**
 * @file transposition_tables.h
 * @brief transposition table
 * @author Philip Brand
 * @date 2025-25-07
 */

#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <stdint.h>

#include "../movefinding/board.h"

extern ULL zobrist_key_table[2][6][64];
extern ULL zobrist_black_to_move;
extern ULL zobrist_en_passant[65];
extern ULL zobrist_castling[2][2];

typedef enum {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
} NodeType_t;

typedef struct {
    ULL zobrist_key;
    ULL best_move_zobrist_key;
    int32_t position_evaluation;
    uint16_t half_move_count;
    uint8_t search_depth;
    NodeType_t node_type;
} TranspositionEntry_t;

typedef struct
{
    ULL zobrist_key;
    bool is_taken;
} PastMoveEntry_t;

#define TT_SIZE_BITS 28
#define TT_SIZE (1ULL << TT_SIZE_BITS)
#define TT_MASK (TT_SIZE - 1)

#define PAST_MOVE_LIST_SIZE_BITS 20
#define PAST_MOVE_LIST_SIZE (1ULL << PAST_MOVE_LIST_SIZE_BITS)
#define PAST_MOVE_LIST_MASK (PAST_MOVE_LIST_SIZE - 1)

extern TranspositionEntry_t *transposition_table;
extern PastMoveEntry_t* past_move_list;

/**
 * @brief Initialises the Zobrist key table with random values.
 */
void zobrist_key_init(void);

/**
 * @brief Initialises the hash tables for transposition and past move list.
 */
void hash_table_init(void);

/**
 * @brief Frees the memory allocated for the transposition table and past move list.
 */
void hash_table_free(void);

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

