#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"
// #include "lookuptables.h"
#include "magic_numbers.h"


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


Position_t fen_to_board(char* fen)
{   
    Position_t fen_position = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    size_t i = 0;  // i does not necessarily count up to 64
    uint8_t square_counter = 0;  // square_counter counts up to 64

    char character = fen[i++];

    while (character != ' ')  // stops and end of board representation not complete FEN
    {
        if (isalpha(character)) {
            if (isupper(character)) {
                if (character == 'K') {
                    fen_position.white_king |= (1ULL << square_counter);
                } else if (character == 'Q') {
                    fen_position.white_queen |= (1ULL << square_counter);
                } else if (character == 'R') {
                    fen_position.white_rooks |= (1ULL << square_counter);
                } else if (character == 'B') {
                    fen_position.white_bishops |= (1ULL << square_counter);
                } else if (character == 'N') {
                    fen_position.white_knights |= (1ULL << square_counter);
                } else if (character == 'P') {
                    fen_position.white_pawns |= (1ULL << square_counter);
                }
                fen_position.white_pieces |= (1ULL << square_counter);
            } else {
                if (character == 'k') {
                    fen_position.black_king |= (1ULL << square_counter);
                } else if (character == 'q') {
                    fen_position.black_queen |= (1ULL << square_counter);
                } else if (character == 'r') {
                    fen_position.black_rooks |= (1ULL << square_counter);
                } else if (character == 'b') {
                    fen_position.black_bishops |= (1ULL << square_counter);
                } else if (character == 'n') {
                    fen_position.black_knights |= (1ULL << square_counter);
                } else if (character == 'p') {
                    fen_position.black_pawns |= (1ULL << square_counter);
                }
                fen_position.black_pieces |= (1ULL << square_counter);
            }
            fen_position.all_pieces |= (1ULL << square_counter);
            square_counter++;
        }
        else if (isdigit(character)) {
            square_counter += character - '0';
        } 
        character = fen[i++];
    }
    return fen_position;
}


void debug_bitboards(void)
{
    generate_lookup_tables();
    print_bitboard(king_attack_lookup_table[18]);
    printf("Magic number for square 18: %llu\n", actual_rook_magic_numbers[18]);
    printf("Blocker mask for square 18: %llu\n", rook_blocker_masks[18]);
    printf("Move mask for square 18: %llu\n", rook_attack_lookup_table[18][0]);
}