#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"
#include "../search/piece.h"

#define WHITE_INDEX 1

char *pretty_print_moves[64] =
    {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

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
        if (i % 8 == 0)
        {
            // pretty printing for the ranks:
            printf("\n");
            printf("%u   ", 8 - i / 8);
        }
        if (bitboard & (1ULL << i))
        {
            printf("1 ");
        }
        else
        {
            printf(". ");
        }
    }
    // pretty printing for the files:
    printf("\n\n    a b c d e f g h\n\n");
}

void print_position(Position_t const position)
{
    char mailboxes[64] = {0};
    bool white_to_move = position.white_to_move;
    PiecesOneColour_t white_pieces = position.pieces[white_to_move];
    PiecesOneColour_t black_pieces = position.pieces[!white_to_move];

    for (uint8_t i = 0; i < 64; i++)
    {
        if (white_pieces.pawns & (1ULL << i))
        {
            mailboxes[i] = 'P';
        }
        else if (white_pieces.knights & (1ULL << i))
        {
            mailboxes[i] = 'N';
        }
        else if (white_pieces.bishops & (1ULL << i))
        {
            mailboxes[i] = 'B';
        }
        else if (white_pieces.rooks & (1ULL << i))
        {
            mailboxes[i] = 'R';
        }
        else if (white_pieces.queens & (1ULL << i))
        {
            mailboxes[i] = 'Q';
        }
        else if (white_pieces.kings & (1ULL << i))
        {
            mailboxes[i] = 'K';
        }
        else if (black_pieces.pawns & (1ULL << i))
        {
            mailboxes[i] = 'p';
        }
        else if (black_pieces.knights & (1ULL << i))
        {
            mailboxes[i] = 'n';
        }
        else if (black_pieces.bishops & (1ULL << i))
        {
            mailboxes[i] = 'b';
        }
        else if (black_pieces.rooks & (1ULL << i))
        {
            mailboxes[i] = 'r';
        }
        else if (black_pieces.queens & (1ULL << i))
        {
            mailboxes[i] = 'q';
        }
        else if (black_pieces.kings & (1ULL << i))
        {
            mailboxes[i] = 'k';
        }
        else
        {
            mailboxes[i] = '.';
        }
    }

    for (uint8_t i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n%d   ", 8 - i / 8);
        }
        printf("%c ", mailboxes[i]);
    }
    printf("\n\n    a b c d e f g h\n");

    printf("\n");
    printf("piece value difference: %d\n", position.piece_value_diff);
    printf("\n");
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
            if (isupper(character))
            {
                pieces = &fen_position->pieces[WHITE_INDEX];
            }
            else
            {
                pieces = &fen_position->pieces[!WHITE_INDEX];
            }
            character = tolower(character);
            if (character == 'k')
            {
                pieces->kings |= (1ULL << square_counter);
            }
            else if (character == 'q')
            {
                pieces->queens |= (1ULL << square_counter);
            }
            else if (character == 'r')
            {
                pieces->rooks |= (1ULL << square_counter);
            }
            else if (character == 'b')
            {
                pieces->bishops |= (1ULL << square_counter);
            }
            else if (character == 'n')
            {
                pieces->knights |= (1ULL << square_counter);
            }
            else if (character == 'p')
            {
                pieces->pawns |= (1ULL << square_counter);
            }
            pieces->all_pieces |= (1ULL << square_counter);
            fen_position->all_pieces |= (1ULL << square_counter);
            square_counter++;
        }
        else if (isdigit(character))
        {
            square_counter += character - '0';
        }
        character = fen[i++];
    }

    // ========================= WHOSE TURN =========================
    if (fen[i++] == 'w')
    {
        fen_position->white_to_move = true;
    }
    else
    {
        fen_position->white_to_move = false;
    }

    // ========================= CASTLING =========================
    i++; // skip space
    character = fen[i++];
    while (character != ' ')
    {
        if (character == 'K')
        {
            fen_position->pieces[WHITE_INDEX].castle_kingside = true;
        }
        else if (character == 'Q')
        {
            fen_position->pieces[WHITE_INDEX].castle_queenside = true;
        }
        else if (character == 'k')
        {
            fen_position->pieces[!WHITE_INDEX].castle_kingside = true;
        }
        else if (character == 'q')
        {
            fen_position->pieces[!WHITE_INDEX].castle_queenside = true;
        }
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
}

void print_children_positions(Position_t *position)
{
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        printf("Child %d:\n", i);
        print_position(*position->child_positions[i]);
    }
}

