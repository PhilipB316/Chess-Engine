#include <stdint.h>
#include <stdbool.h>

#include "lookuptables.h"

void pawn_attack_generator(ULL pawn_attack_lookup_table[2][64])
{
    // white attacks (heading north, starting from square 55)
    for (int i = 55; i > 8; i--)
    {
        ULL pawn = 1ULL << i;
        ULL pawn_attack = 0;
        // masks
        ULL left_attack = (pawn & ~FILE_A) >> 9;
        ULL right_attack = (pawn & ~FILE_H) >> 7;
        pawn_attack |= left_attack | right_attack;
        pawn_attack_lookup_table[1][i] = pawn_attack;
    }
    // black attacks (heading south, starting from square 7)
    for (int i = 7; i < 56; i++)
    {
        ULL pawn = 1ULL << i;
        ULL pawn_attack = 0;
        ULL left_attack = (pawn & ~FILE_A) << 7;
        ULL right_attack = (pawn & ~FILE_H) << 9;
        pawn_attack |= left_attack | right_attack;
        pawn_attack_lookup_table[2][i] = pawn_attack;
    }
}



void knight_attack_generator(ULL knight_attack_lookup_table[64])
{
    for (int i = 0; i < 64; i++)
    {
        ULL knight = 1ULL << i;
        ULL knight_attack = 0;

        // masks to prevent wraparound
        ULL hard_left_up = (knight & ~FILE_A & ~FILE_B & ~RANK_1) >> 10;
        ULL soft_left_up = (knight & ~FILE_A & ~RANK_1 & ~RANK_2) >> 17;
        ULL hard_right_up = (knight & ~FILE_H & ~FILE_G & ~RANK_1) >> 6;
        ULL soft_right_up = (knight & ~FILE_H & ~RANK_1 & ~RANK_2) >> 15;

        ULL hard_left_down = (knight & ~FILE_A & ~FILE_B & ~RANK_8) << 6;
        ULL soft_left_down = (knight & ~FILE_A & ~RANK_7 & ~RANK_8) << 15;
        ULL hard_right_down = (knight & ~FILE_H & ~FILE_G & ~RANK_8) << 10;
        ULL soft_right_down = (knight & ~FILE_H & ~RANK_7 & ~RANK_8) << 17;

        knight_attack |= hard_left_up
                        | soft_left_up
                        | hard_right_up
                        | soft_right_up
                        | hard_left_down
                        | soft_left_down
                        | hard_right_down
                        | soft_right_down;

        knight_attack_lookup_table[i] = knight_attack;
    }
}


void generate_lookup_tables(
    ULL pawn_attack_lookup_table[2][64], 
    ULL knight_attack_lookup_table[64])
{
    pawn_attack_generator(pawn_attack_lookup_table);
    knight_attack_generator(knight_attack_lookup_table);
}