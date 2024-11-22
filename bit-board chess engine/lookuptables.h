#ifndef LOOKUPTABLES_H
#define LOOKUPTABLES_H

#include <stdint.h>

#define ULL unsigned long long

// files and ranks used to prevent wraparound
#define FILE_A 0x0101010101010101
#define FILE_B 0x0202020202020202
#define FILE_G 0x4040404040404040
#define FILE_H 0x8080808080808080

#define RANK_1 0x00000000000000FF
#define RANK_2 0x000000000000FF00
#define RANK_7 0x00FF000000000000
#define RANK_8 0xFF00000000000000

static ULL pawn_attack_lookup_table[2][64];
static ULL knight_attack_lookup_table[64];
static ULL king_attack_lookup_table[64];

typedef struct {
    ULL pawn_attack_bitboard[2][64];
    ULL knight_attack_bitboard[64];
    ULL king_attack_bitboard[64];
} AttackLookupTables_t;



/**
 * @brief Generates a lookup table for pawn attacks.
 * 
 * This function populates a lookup table that provides precomputed attack
 * bitboards for pawns. The table is indexed by pawn color (0 for white, 1 for black)
 * and the square index (0-63) representing the position of the pawn on the board.
 * 
 * @param pawn_attack_lookup_table A 2D array to be filled with bitboards indicating
 *        the attack positions for pawns.
 */
void pawn_attack_generator(ULL pawn_attack_lookup_table[2][64]);


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
void knight_attack_generator(ULL knight_attack_lookup_table[64]);




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
void king_attack_generator(ULL king_attack_lookup_table[64]);


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
void generate_lookup_tables(AttackLookupTables_t *lookup_tables);



#endif