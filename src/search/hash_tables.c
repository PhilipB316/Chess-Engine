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

ULL past_move_stack[
    MAXIMUM_GAME_LENGTH + MAX_SEARCH_DEPTH + MAX_QUIESCENCE_DEPTH];
int past_move_stack_top = 0;

ULL random_64_bit(void)
{
    // set unique seed:
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    return ((ULL)rand() << 33) | ((ULL)rand() << 2) | (rand() & 3);
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

    ULL bitboard = position->en_passant_bitboard;
    if (__builtin_expect(bitboard, 0)) {
        hash ^= zobrist_en_passant[__builtin_ctzll(bitboard)];
    }

    // Loop over both colors: 0 = white, 1 = black
    for (int colour = 0; colour < 2; colour++) {
        PieceType_t piece_types[] = {
            PIECE_PAWN, PIECE_KNIGHT, PIECE_BISHOP, PIECE_ROOK, PIECE_QUEEN
        };
        ULL* bitboards[] = {
            &position->pieces[colour].pawns,
            &position->pieces[colour].knights,
            &position->pieces[colour].bishops,
            &position->pieces[colour].rooks,
            &position->pieces[colour].queens
        };
        for (int i = 0; i < 5; ++i) {
            ULL bitboard = *bitboards[i];
            while (bitboard) {
                uint8_t square = __builtin_ctzll(bitboard);
                hash ^= zobrist_key_table[colour][piece_types[i]][square];
                bitboard &= bitboard - 1;
            }
        }
        // King (assume only one king per color)
        if (position->pieces[colour].kings) {
            uint8_t king_square = __builtin_ctzll(position->pieces[colour].kings);
            hash ^= zobrist_key_table[colour][PIECE_KING][king_square];
        }
    }

    if (position->pieces[white_to_move].castle_kingside) 
    { hash ^= zobrist_castling[white_to_move][KINGSIDE]; }
    if (position->pieces[white_to_move].castle_queenside) 
    { hash ^= zobrist_castling[white_to_move][QUEENSIDE]; }
    if (position->pieces[!white_to_move].castle_kingside)
    { hash ^= zobrist_castling[!white_to_move][KINGSIDE]; }
    if (position->pieces[!white_to_move].castle_queenside)
    { hash ^= zobrist_castling[!white_to_move][QUEENSIDE]; }

    return hash;
}

bool is_threefold_repetition(Position_t* position)
{
    int needed = 2;
    ULL key = position->zobrist_key;
    for (int i = past_move_stack_top - 1; i >= 0; i--) {
        if (past_move_stack[i] == key && --needed == 0) return true;
        if ((i+1) < needed) break;
    }
    return false;
}

void hash_table_init(void) 
{
    transposition_table = calloc(1, sizeof(TranspositionEntry_t) * TT_SIZE);
    if (!transposition_table) {
        fprintf(stderr, "Failed to allocate transposition table\n");
        exit(1);
    }
}

void hash_table_free(void)
{ free(transposition_table); }

