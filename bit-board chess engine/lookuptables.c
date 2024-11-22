#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>  

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
        // 1 index for white, 0 index for black
        pawn_attack_lookup_table[1][i] = pawn_attack;
    }
    // black attacks (heading south, starting from square 7)
    for (int i = 7; i < 56; i++)
    {
        ULL pawn = 1ULL << i;
        ULL pawn_attack = 0;
        // masks
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
        // printf("knight_attack_lookup_table[%d]: %llu\n", i, knight_attack_lookup_table[i]);
    }
}


void king_attack_generator(ULL king_attack_lookup_table[64])
{
    for (int i = 0; i < 64; i++)
    {
        ULL king = 1ULL << i;
        ULL king_attack = 0;

        // masks to prevent wraparound
        ULL left = (king & ~FILE_A) >> 1;
        ULL right = (king & ~FILE_H) << 1;
        ULL up = (king & ~RANK_1) >> 8;
        ULL down = (king & ~RANK_8) << 8;

        ULL left_up = (king & ~FILE_A & ~RANK_1) >> 9;
        ULL right_up = (king & ~FILE_H & ~RANK_1) >> 7;
        ULL left_down = (king & ~FILE_A & ~RANK_8) << 7;
        ULL right_down = (king & ~FILE_H & ~RANK_8) << 9;

        king_attack |= left
                    | right
                    | up
                    | down
                    | left_up
                    | right_up
                    | left_down
                    | right_down;

        king_attack_lookup_table[i] = king_attack;
    }
}


void generate_lookup_tables(AttackLookupTables_t *lookup_tables)
{
    pawn_attack_generator(lookup_tables->pawn_attack_bitboard);
    knight_attack_generator(lookup_tables->knight_attack_bitboard);
    king_attack_generator(lookup_tables->king_attack_bitboard);
}