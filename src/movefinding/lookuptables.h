/**
 * @file move_lookuptables.h
 * @brief Implementation of bitboards lookuptables for piece attacks.
 * @author Philip Brand
 * @date 2024-11-29
 *
 * The lookuptables for all pieces are set as extern, so once generated using
 * the generate_lookup_tables(), the arrays are available to all other files.
 *
 * TODO: Possibly implement magic numbers for a lookup table for two knights:
 * - instead of determining the possible moves for each knight individually, lookup
 * both at the same time using a magic index.
 */

#ifndef LOOKUPTABLES_H
#define LOOKUPTABLES_H

#include <stdint.h>

#define ULL unsigned long long
#define KNOWN_MAGIC_NUMBERS 1
#define QUEENSIDE 1

// files and ranks used to prevent wraparound
#define FILE_A 0x0101010101010101
#define FILE_B 0x0202020202020202
#define FILE_G 0x4040404040404040
#define FILE_H 0x8080808080808080

#define RANK_1 0x00000000000000FF
#define RANK_2 0x000000000000FF00
#define RANK_7 0x00FF000000000000
#define RANK_8 0xFF00000000000000

// lookup tables for piece attacks
extern ULL pawn_attack_lookup_table[2][64];
extern ULL knight_attack_lookup_table[64];
extern ULL king_attack_lookup_table[64];
extern ULL rook_attack_lookup_table[64][4096];
extern ULL bishop_attack_lookup_table[64][4096];
// extern ULL magic_knight_attack_lookup_table[4096];

// masks to determine relevant blockers for rooks and bishops and castling
extern ULL rook_blocker_masks[64];
extern ULL bishop_blocker_masks[64];
extern ULL castling_blocker_masks[2][3];

extern ULL rook_castling_array[2][2];
extern ULL king_castling_array[2][2];

// (64 - bits) numbers of bits rook/ bishop blockers can occupy
extern uint8_t offset_RBits[64];
extern uint8_t offset_BBits[64];

// prefound magic numbers for bishops and rooks
extern ULL actual_bishop_magic_numbers[64];
extern ULL actual_rook_magic_numbers[64];

/**
 * @brief Generates the lookup tables for rook attacks using magic_numbers module
 * @param rook_attack_lookup_table the table to be populated
 */
void rook_attack_generator(void);

/**
 * @brief Generates the lookup tables for bishop attacks using magic_numbers module
 * @param bishop_attack_lookup_table the table to be populated
 */
void bishop_attack_generator(void);

/**
 * @brief Generates a lookup table for pawn attacks.
 *
 * This function populates a lookup table that provides precomputed attack
 * bitboards for pawns. The table is indexed by pawn color (0 for white, 1 for black)
 * and the square index (0-63) representing the position of the pawn on the board.

 */
void pawn_attack_generator(void);

/**
 * @brief Generates a lookup table for knight attacks.
 *
 * This function populates a lookup table that provides precomputed attack
 * bitboards for knights. The table is indexed by the square index (0-63) representing
 * the position of the knight on the board.
 *
 * @param knight_attack_lookup_table An array to be filled with bitboards indicating
 *        the attack positions for knights.
 */
void knight_attack_generator(void);

/**
 * @brief Generates lookup tables for king attacks.
 *
 * This function populates a lookup table that provides precomputed attack
 * bitboards for kings. The table is indexed by the square index (0-63) representing
 * the position of the king on the board.
 *
 * @param king_attack_lookup_table An array to be filled with bitboards indicating
 *       the attack positions for kings.
 */
void king_attack_generator(void);

/**
 * @brief Generates lookup tables for piece attacks.
 *
 * This function initializes the provided lookup tables with precomputed
 * attack patterns - the lookup tables are used to quickly determine
 * the possible moves for these pieces from any given position.
 *
 * @param lookup_tables The lookup tables to be filled with precomputed
 *  attack patterns.
 **/
void generate_lookup_tables(void);

#endif
