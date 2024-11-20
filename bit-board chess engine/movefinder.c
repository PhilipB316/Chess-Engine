#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"

void print_bitboard(uint64_t bitboard)
{
    for (size_t i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n");
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
}