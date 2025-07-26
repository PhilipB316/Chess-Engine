// transposition_table.c

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "transposition_table.h"

ULL zobrist_key_table[2][6][64];
ULL zobrist_black_to_move;
ULL zobrist_en_passant[65];
ULL zobrist_castling[2][2];

ULL random_64_bit(void)
{
    return ((ULL)rand() << 32) | rand();
}

void zobrist_key_init(void)
{
    for (int colour = 0; colour < 2; colour++) {
        for (int piece = 0; piece < 6; piece++) {
            for (int square = 0; square < 64; square++) {
                zobrist_key_table[colour][piece][square] = random_64_bit();
            }
        }
    }

    for (int square = 0; square < 64; square++) {
        zobrist_en_passant[square] = random_64_bit(); }
    zobrist_en_passant[64] = 0ULL; // No en passant on the last square
    zobrist_en_passant[0] = 0ULL; // No en passant on the first square

    for (int colour = 0; colour < 2; colour++) {
        for (int castling_rights = 0; castling_rights < 2; castling_rights++) {
            zobrist_castling[colour][castling_rights] = random_64_bit();
        }
    }

    zobrist_black_to_move = random_64_bit();
}

ULL generate_zobrist_hash(Position_t *position)
{
    ULL hash = 0;
    bool white_to_move = position->white_to_move;

    hash ^= zobrist_black_to_move * !white_to_move;
    hash ^= zobrist_en_passant[__builtin_ctzll(position->en_passant_bitboard)];

    // Loop over both colors: 0 = white, 1 = black
    for (int colour = 0; colour < 2; colour++) {
        // Pawns
        ULL bitboard = position->pieces[colour].pawns;
        while (bitboard) {
            uint8_t square = __builtin_ctzll(bitboard);
            hash ^= zobrist_key_table[colour][PIECE_PAWN][square];
            bitboard &= bitboard - 1;
        }
        // Knights
        bitboard = position->pieces[colour].knights;
        while (bitboard) {
            uint8_t square = __builtin_ctzll(bitboard);
            hash ^= zobrist_key_table[colour][PIECE_KNIGHT][square];
            bitboard &= bitboard - 1;
        }
        // Bishops
        bitboard = position->pieces[colour].bishops;
        while (bitboard) {
            uint8_t square = __builtin_ctzll(bitboard);
            hash ^= zobrist_key_table[colour][PIECE_BISHOP][square];
            bitboard &= bitboard - 1;
        }
        // Rooks
        bitboard = position->pieces[colour].rooks;
        while (bitboard) {
            uint8_t square = __builtin_ctzll(bitboard);
            hash ^= zobrist_key_table[colour][PIECE_ROOK][square];
            bitboard &= bitboard - 1;
        }
        // Queens
        bitboard = position->pieces[colour].queens;
        while (bitboard) {
            uint8_t square = __builtin_ctzll(bitboard);
            hash ^= zobrist_key_table[colour][PIECE_QUEEN][square];
            bitboard &= bitboard - 1;
        }
        // King (assume only one king per color)
        if (position->pieces[colour].kings) {
            uint8_t king_square = __builtin_ctzll(position->pieces[colour].kings);
            hash ^= zobrist_key_table[colour][PIECE_KING][king_square];
        }
    }

    if (position->pieces[white_to_move].castle_kingside) {
        hash ^= zobrist_castling[white_to_move][KINGSIDE];
    }
    if (position->pieces[white_to_move].castle_queenside) {
        hash ^= zobrist_castling[white_to_move][QUEENSIDE];
    }
    if (position->pieces[!white_to_move].castle_kingside) {
        hash ^= zobrist_castling[!white_to_move][KINGSIDE];
    }
    if (position->pieces[!white_to_move].castle_queenside) {
        hash ^= zobrist_castling[!white_to_move][QUEENSIDE];
    }

    return hash;
}

