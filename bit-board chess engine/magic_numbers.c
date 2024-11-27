#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "lookuptables.h"
#include "movefinder.h"

static ULL rook_blocker_masks[64];
static ULL bishop_blocker_masks[64];

static ULL rook_magic_numbers[64];
static ULL rook_attack_lookup_table[64][4096];

uint8_t RBits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};


ULL random_ULL() {
    ULL u1, u2, u3, u4;
    // srand(time(NULL));
    u1 = (ULL)(random()) & 0xFFFF;
    u2 = (ULL)(random()) & 0xFFFF;
    u3 = (ULL)(random()) & 0xFFFF; 
    u4 = (ULL)(random()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}


ULL random_ULL_fewbits() {
    return random_ULL() & random_ULL() & random_ULL();
}


void generate_rook_blocker_masks(ULL rook_blocker_masks[64])
{
    for (int square = 0; square < 64; square++)
    {
        ULL mask = 0;
        int square_rank = square / 8;
        int square_file = square % 8;

        // north
        for (int i = square_rank + 1; i < 7; i++)
        {
            mask |= (1ULL << (square_file + i * 8));
        }
        // south
        for (int i = square_rank - 1; i > 0; i--)
        {
            mask |= (1ULL << (square_file + i * 8));
        }
        // east
        for (int i = square_file + 1; i < 7; i++)
        {
            mask |= (1ULL << (i + square_rank * 8));
        }
        // west
        for (int i = square_file - 1; i > 0; i--)
        {
            mask |= (1ULL << (i + square_rank * 8));
        }

        rook_blocker_masks[square] = mask;   
    }
}


ULL determine_possible_rook_moves(uint8_t sq, ULL blocker) {
  ULL result = 0ULL;
  uint8_t square_rank = sq/8; 
  uint8_t square_file = sq%8;
  int rank, file;
  uint8_t possible_square;
  for (rank = square_rank+1; rank <= 7; rank++) {
    possible_square = square_file + rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
  }
  for (rank = square_rank-1; rank >= 0; rank--) {
    possible_square = square_file + rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
  }
  for (file = square_file+1; file <= 7; file++) {
    possible_square = file + square_rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
  }
  for (file = square_file-1; file >= 0; file--) {
    possible_square = file + square_rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
  }
  return result;
}


bool generate_possible_rook_blockers(uint8_t square)
{
    ULL mask = rook_blocker_masks[square];
    int bits = __builtin_popcountll(mask);
    int permutations = 1 << bits;
    ULL magic_number = random_ULL_fewbits();
    static ULL counter = 0;

    for (int i = 0; i < permutations; i++)
    {
        ULL blocker = 0;
        int bit_index = 0;
        for (int j = 0; j < 64; j++)
        {
            if (mask & (1ULL << j))
            {
                if (i & (1 << bit_index))
                {
                    blocker |= (1ULL << j);
                }
                bit_index++;
            }
        }

        ULL possible_moves = determine_possible_rook_moves(square, blocker);
        ULL index = (blocker * magic_number) >> (64 - bits);
        ULL current = rook_attack_lookup_table[square][index];
        if (current == 0 || current == possible_moves) {
            rook_attack_lookup_table[square][index] = possible_moves;
        } else{
            counter++;
            return 0;
        }
    }
    printf("Collision detected %llu times\n", counter);
    rook_magic_numbers[square] = magic_number;
    return 1;
}


int main(void)
{
    generate_rook_blocker_masks(rook_blocker_masks);
    printf("Generated masks\n");
    for (int j = 0; j < 64; j++)
    {
        printf("Square %d:\n", j);
        while (!generate_possible_rook_blockers(j))
        {
            for (int i = 0; i < 4096; i++)
            {
                rook_attack_lookup_table[j][i] = 0;
            }
        }
    }

    for (int i = 0; i < 64; i++)
    {   
        printf("Square: %d\n", i);
        ULL random_position = random_ULL();
        print_bitboard(random_position);
        ULL blockers = rook_blocker_masks[i] & random_position;
        print_bitboard(blockers);

        uint8_t bits = RBits[i];
        ULL index = (blockers * rook_magic_numbers[i]) >> (64 - bits);
        printf("Moves: \n");
        print_bitboard(rook_attack_lookup_table[i][index]);
        srand(index);
        printf("\n\n");

    }

    // for (int i = 0; i < 64; i++)
    // {
    //     printf("Rook magic number for square %d: %llu\n", i, rook_magic_numbers[i]);
    // }
}