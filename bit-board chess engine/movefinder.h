#ifndef MOVEFINDER_H
#define MOVEFINDER_H

#include <stdint.h>

#define ULL unsigned long long

typedef struct {
    ULL black_king;
    ULL black_queen;
    ULL black_rooks;
    ULL black_bishops;
    ULL black_knights;
    ULL black_pawns;
    ULL black_pieces;

    ULL white_king;
    ULL white_queen;
    ULL white_rooks;
    ULL white_bishops;
    ULL white_knights;
    ULL white_pawns;
    ULL white_pieces;

    ULL all_pieces;
} Position_t;


/**
 * @brief Converts a FEN (Forsyth-Edwards Notation) string to a position,
 * represented as bitboards.
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


void debug_bitboards(void);

#endif