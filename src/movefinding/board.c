#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "board.h"
#include "lookuptables.h"
#include "../search/evaluate.h"
#include "../search/hash_tables.h"

char *pretty_print_moves[64] =
    {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
    };

int16_t calculate_piece_value_diff(Position_t *position)
{
    int16_t piece_value_diff = 0;
    PiecesOneColour_t *white_pieces = &position->pieces[WHITE_INDEX];
    PiecesOneColour_t *black_pieces = &position->pieces[!WHITE_INDEX];

    piece_value_diff += __builtin_popcountll(white_pieces->pawns) * PAWN_VALUE;
    piece_value_diff -= __builtin_popcountll(black_pieces->pawns) * PAWN_VALUE;

    piece_value_diff += __builtin_popcountll(white_pieces->knights) * KNIGHT_VALUE;
    piece_value_diff -= __builtin_popcountll(black_pieces->knights) * KNIGHT_VALUE;

    piece_value_diff += __builtin_popcountll(white_pieces->bishops) * BISHOP_VALUE;
    piece_value_diff -= __builtin_popcountll(black_pieces->bishops) * BISHOP_VALUE;

    piece_value_diff += __builtin_popcountll(white_pieces->rooks) * ROOK_VALUE;
    piece_value_diff -= __builtin_popcountll(black_pieces->rooks) * ROOK_VALUE;

    piece_value_diff += __builtin_popcountll(white_pieces->queens) * QUEEN_VALUE;
    piece_value_diff -= __builtin_popcountll(black_pieces->queens) * QUEEN_VALUE;

    piece_value_diff += __builtin_popcountll(white_pieces->kings) * KING_VALUE;
    piece_value_diff -= __builtin_popcountll(black_pieces->kings) * KING_VALUE;

    return piece_value_diff;
}

void print_bitboard(uint64_t bitboard)
{
    for (uint8_t i = 0; i < 64; i++)
    {
        if (i % 8 == 0) { printf("\n%u   ", 8 - i / 8); }
        if (bitboard & (1ULL << i)) { printf("1 "); }
        else { printf(". "); }
    }
    printf("\n\n    a b c d e f g h\n\n");
}


void fen_to_board(char *fen, Position_t *fen_position)
{
    memset(fen_position, 0, sizeof(Position_t));
    size_t i = 0; // i does not necessarily count up to 64
    uint8_t square_counter = 0; // square_counter counts up to 64
    char character = fen[i++];

    // ========================= PIECES =========================
    while (character != ' ')
    {
        PiecesOneColour_t *pieces;
        if (isalpha(character))
        {
            if (isupper(character)) { pieces = &fen_position->pieces[WHITE_INDEX]; }
            else { pieces = &fen_position->pieces[!WHITE_INDEX]; }

            character = tolower(character);
            if (character == 'k') { pieces->kings |= (1ULL << square_counter); }
            else if (character == 'q') { pieces->queens |= (1ULL << square_counter); }
            else if (character == 'r') { pieces->rooks |= (1ULL << square_counter); }
            else if (character == 'b') { pieces->bishops |= (1ULL << square_counter); }
            else if (character == 'n') { pieces->knights |= (1ULL << square_counter); }
            else if (character == 'p') { pieces->pawns |= (1ULL << square_counter); }

            pieces->all_pieces |= (1ULL << square_counter);
            fen_position->all_pieces |= (1ULL << square_counter);
            square_counter++;
        }
        else if (isdigit(character)) { square_counter += character - '0'; }
        character = fen[i++];
    }

    // ========================= WHOSE TURN =========================
    if (fen[i++] == 'w') { fen_position->white_to_move = true; }
    else { fen_position->white_to_move = false; }

    // ========================= CASTLING =========================
    i++; // skip space
    character = fen[i++];
    while (character != ' ')
    {
        if (character == 'K') { fen_position->pieces[WHITE_INDEX].castle_kingside = true; }
        else if (character == 'Q') { fen_position->pieces[WHITE_INDEX].castle_queenside = true; }
        else if (character == 'k') { fen_position->pieces[!WHITE_INDEX].castle_kingside = true; }
        else if (character == 'q') { fen_position->pieces[!WHITE_INDEX].castle_queenside = true; }
        character = fen[i++];
    }

    // ========================= EN PASSANT =========================
    // skip space
    character = fen[i++];
    if (character != '-')
    {
        uint8_t file = character - 'a';
        uint8_t rank = 8 - (fen[i++] - '0');
        fen_position->en_passant_bitboard = 1ULL << (8 * rank + file);
    }
    else
    {
        fen_position->en_passant_bitboard = 0;
    }
    fen_position->parent_position = NULL;
    fen_position->num_children = 0;
    fen_position->piece_value_diff = calculate_piece_value_diff(fen_position);

    // ========================== HALF MOVE COUNT =========================
    i++; // skip space
    uint8_t half_move_count = 0;
    while (isdigit(fen[i])) { half_move_count = half_move_count * 10 + (fen[i++] - '0'); }

    // ========================= WHOLE MOVES =========================
    i++; // skip space
    uint8_t whole_move_count = 0;
    while (isdigit(fen[i])) { whole_move_count = whole_move_count * 10 + (fen[i++] - '0'); }
    fen_position->half_move_count = whole_move_count * 2 - 1;
    if (fen_position->white_to_move) { fen_position->half_move_count--; }

    fen_position->zobrist_key = generate_zobrist_hash(fen_position);
}

bool is_different(Position_t* position1, Position_t* position2)
{
    if (position1->white_to_move != position2->white_to_move) return true;
    if (position1->en_passant_bitboard != position2->en_passant_bitboard) return true;
    if (position1->all_pieces != position2->all_pieces) return true;

    for (int i = 0; i < 2; i++)
    {
        PiecesOneColour_t* pieces1 = &position1->pieces[i];
        PiecesOneColour_t* pieces2 = &position2->pieces[i];

        if (pieces1->castle_kingside != pieces2->castle_kingside) return true;
        if (pieces1->castle_queenside != pieces2->castle_queenside) return true;
        if (pieces1->pawns != pieces2->pawns) return true;
        if (pieces1->knights != pieces2->knights) return true;
        if (pieces1->bishops != pieces2->bishops) return true;
        if (pieces1->rooks != pieces2->rooks) return true;
        if (pieces1->queens != pieces2->queens) return true;
        if (pieces1->kings != pieces2->kings) return true;
    }
    return false;
}

void find_from_to_square(Position_t* position1,
                         Position_t* position2,
                         ULL* from_bitboard,
                         ULL* to_bitboard)
{
    bool white_to_move = position1->white_to_move;
    ULL move_bitboard = position1->pieces[white_to_move].all_pieces ^ position2->pieces[white_to_move].all_pieces;
    uint8_t moved_piece_count = 0;

    *from_bitboard = move_bitboard & position1->pieces[white_to_move].all_pieces;
    *to_bitboard = move_bitboard & position2->pieces[white_to_move].all_pieces;

    while (move_bitboard) {
        moved_piece_count++;
        move_bitboard &= ~(1ULL << __builtin_ctzll(move_bitboard));
    }
    if (moved_piece_count > 2) {
        *from_bitboard = position1->pieces[white_to_move].kings;
        *to_bitboard = position2->pieces[white_to_move].kings;
    }
}

bool piece_moved_more_than_one_square(ULL from_bitboard, ULL to_bitboard)
{
    uint8_t from_square = __builtin_ctzll(from_bitboard);
    uint8_t to_square = __builtin_ctzll(to_bitboard);
    return (abs((int)(from_square / 8) - (int)(to_square / 8)) > 1 ||
            abs((int)(from_square % 8) - (int)(to_square % 8)) > 1);
}

MoveType_t find_move_type(Position_t *from_position, Position_t *to_position)
{
    ULL from_bitboard, to_bitboard;
    PiecesOneColour_t from_active_pieces_set = from_position->pieces[from_position->white_to_move];
    PiecesOneColour_t to_active_pieces_set = to_position->pieces[from_position->white_to_move];
    find_from_to_square(from_position, to_position, &from_bitboard, &to_bitboard);

    if (from_bitboard & from_active_pieces_set.pawns)
    {
        if (to_bitboard & from_active_pieces_set.pawns) { return PAWN; }
        else if (to_bitboard & to_active_pieces_set.bishops) { return PROMOTE_BISHOP; }
        else if (to_bitboard & to_active_pieces_set.rooks) { return PROMOTE_ROOK; }
        else if (to_bitboard & to_active_pieces_set.queens) { return PROMOTE_QUEEN; }
        else if (to_bitboard & to_active_pieces_set.kings) { return PROMOTE_KNIGHT; }
    }
    else if (from_bitboard & from_active_pieces_set.knights) { return KNIGHT; }
    else if (from_bitboard & from_active_pieces_set.bishops) { return BISHOP; }
    else if (from_bitboard & from_active_pieces_set.rooks) { return ROOK; }
    else if (from_bitboard & from_active_pieces_set.queens) { return QUEEN; }
    else if (from_bitboard & from_active_pieces_set.kings)
    {
        if (to_bitboard & king_castling_array[from_position->white_to_move][KINGSIDE])
        { return CASTLE_KINGSIDE; }
        else if (to_bitboard & king_castling_array[from_position->white_to_move][QUEENSIDE])
        { return CASTLE_QUEENSIDE; }
        else
        { return KING; }
    }
    return 0; // Invalid move type
}

