#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "lookuptables.h"
#include "movefinder.h"

static ULL rook_blocker_masks[64];
static ULL bishop_blocker_masks[64];

void generate_rook_blocker_masks(ULL rook_blocker_masks[64])
{
    for (int square = 0; square < 64; square++)
    {
        ULL mask = 0;
        int rank = square / 8;
        int file = square % 8;

        // north
        for (int i = rank + 1; i < 7; i++)
        {
            mask |= (1ULL << (file + i * 8));
        }
        // south
        for (int i = rank - 1; i > 0; i--)
        {
            mask |= (1ULL << (file + i * 8));
        }
        // east
        for (int i = file + 1; i < 7; i++)
        {
            mask |= (1ULL << (i + rank * 8));
        }
        // west
        for (int i = file - 1; i > 0; i--)
        {
            mask |= (1ULL << (i + rank * 8));
        }

        rook_blocker_masks[square] = mask;   
    }
}

void generate_possible_rook_blockers(uint8_t square)
{
    ULL mask = rook_blocker_masks[square];
    int bits = __builtin_popcountll(mask);
    int permutations = 1 << bits;
    ULL* blockers[permutations];

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
        // memcpy(blockers[i], &blocker, sizeof(ULL));
        // blockers[i] = blocker;
        printf("Blocker %d:\n", i);
        print_bitboard(blocker);

    }

    // return blockers;
}

// void generate_bishop_blocker_masks(ULL bishop_blocker_masks[64])

int main(void)
{
    generate_rook_blocker_masks(rook_blocker_masks);
    generate_possible_rook_blockers(0);
    // for (int i = 0; i < 10; i += 1)
    // {
    //     print_bitboard(blockers[i]);
    // }
}