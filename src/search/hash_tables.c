// transposition_table.c

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../movefinding/board.h"
#include "hash_tables.h"

ULL zobrist_key_table[2][6][64];
ULL zobrist_black_to_move;
ULL zobrist_en_passant[65];
ULL zobrist_castling[2][2];

TranspositionEntry_t *transposition_table = NULL;
PastMoveEntry_t* past_move_list = NULL;

ULL random_64_bit(void)
{
    // set unique seed:
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
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

void inset_past_move_entry(Position_t* child)
{
    // populate past move list using linear probing
    int32_t index = child->zobrist_key & PAST_MOVE_LIST_MASK;
    bool is_taken = past_move_list[index].is_taken;
    while (is_taken) {
        if (past_move_list[index].zobrist_key == child->zobrist_key) {
            return; // already exists
        } else {
            index = (index + 1) & PAST_MOVE_LIST_MASK;
        }
        is_taken = past_move_list[index].is_taken;
    }
    past_move_list[index].is_taken = true;
    past_move_list[index].zobrist_key = child->zobrist_key;
}

void hash_table_init(void) 
{
    transposition_table = malloc(sizeof(TranspositionEntry_t) * TT_SIZE);
    if (!transposition_table) {
        fprintf(stderr, "Failed to allocate transposition table\n");
        exit(1);
    }

    past_move_list = malloc(sizeof(PastMoveEntry_t) * PAST_MOVE_LIST_SIZE);
    if (!past_move_list) {
        fprintf(stderr, "Failed to allocate past move list\n");
        exit(1);
    }

}

void hash_table_free(void)
{
    free(transposition_table);
    free(past_move_list);
}
