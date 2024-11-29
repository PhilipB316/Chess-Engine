/**
 * @file magic_numbers.c
 * @brief This file contains the implementation of the magic bitboard attack generation.
 * @author Philip Brand
 * @date 2024-11-28
 * 
 * Implementation of the magic bitboard attack generation for rooks and bishops.
 * 
 * The blocker masks are a mask that is applied to all pieces on the board
 * to determine which pieces are blocking the rook or bishop from moving.
 * These pieces are called blockers.
 * 
 * Implementation of the masks is as: relevant_blockers = all_pieces & mask;
 * 
 * The magic numbers create a hash which relates each possible blocker permutation
 * to the possible moves for a piece with that given blocker configuration.
 * 
 * The magic numbers are generate by trial and error
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "magic_numbers.h"


// masks to determine relevant blockers for rooks and bishops
ULL rook_blocker_masks[64];
ULL bishop_blocker_masks[64];

// lookup tables for rook and bishop attacks
ULL rook_attack_lookup_table[64][4096];
ULL bishop_attack_lookup_table[64][4096];
ULL pawn_attack_lookup_table[2][64];
ULL knight_attack_lookup_table[64];
ULL king_attack_lookup_table[64];

// arrays for storing magic numbers for rooks and bishops if looking for them
static ULL array_for_rook_magic_numbers[64];
static ULL array_for_bishop_random_numbers[64];


void pawn_attack_generator(void)
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


void knight_attack_generator(void)
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


void king_attack_generator(void)
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


/**
 * @brief Generates a random unsigned long long integer.
 *
 * This function utilizes the C standard library's random number generation
 * facilities to produce a random unsigned long long integer. It ensures
 * that the generated number is within the range of an unsigned long long.
 *
 * @return A randomly generated unsigned long long integer.
 */
ULL random_ULL(void) 
{
    ULL u1, u2, u3, u4;
    // srand(time(NULL));
    u1 = (ULL)(random()) & 0xFFFF;
    u2 = (ULL)(random()) & 0xFFFF;
    u3 = (ULL)(random()) & 0xFFFF; 
    u4 = (ULL)(random()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}


/**
 * @brief Generates a random unsigned long long integer with a few bits set as 1.
 * @return A random unsigned long long integer with a few bits set.
 */
ULL random_ULL_fewbits(void) 
{
    return random_ULL() & random_ULL() & random_ULL();
}


/**
 * @brief Generates the blocker masks for rooks for all squares on the board.
 * Iterates through each direction until it reaches the edge of the board - although:
 * the actual edge of the board is not necessary as a piece on the very edge has the
 * same effect as the edge of the board.
 */
void generate_rook_blocker_masks(void)
{
    for (int square = 0; square < 64; square++)
    {
        ULL mask = 0;
        int square_rank = square / 8;
        int square_file = square % 8;

        // north direction
        for (int i = square_rank + 1; i < 7; i++)
        {
            mask |= (1ULL << (square_file + i * 8));
        }
        // south direction
        for (int i = square_rank - 1; i > 0; i--)
        {
            mask |= (1ULL << (square_file + i * 8));
        }
        // east direction
        for (int i = square_file + 1; i < 7; i++)
        {
            mask |= (1ULL << (i + square_rank * 8));
        }
        // west direction
        for (int i = square_file - 1; i > 0; i--)
        {
            mask |= (1ULL << (i + square_rank * 8));
        }

        rook_blocker_masks[square] = mask;   
    }
}

void generate_bishop_blocker_masks(void)
{
    for (int square = 0; square < 64; square++)
    {
        ULL mask = 0ULL;
        int square_rank = square / 8;
        int square_file = square % 8;

        // north-east direction
        for (int r = square_rank + 1, f = square_file + 1; r < 7 && f < 7; r++, f++)
        {
            mask |= (1ULL << (f + r * 8));
        }
        // north-west direction
        for (int r = square_rank + 1, f = square_file - 1; r < 7 && f > 0; r++, f--)
        {
            mask |= (1ULL << (f + r * 8));
        }
        // south-east direction
        for (int r = square_rank - 1, f = square_file + 1; r > 0 && f < 7; r--, f++)
        {
            mask |= (1ULL << (f + r * 8));
        }
        // south-west direction
        for (int r = square_rank - 1, f = square_file - 1; r > 0 && f > 0; r--, f--)
        {
            mask |= (1ULL << (f + r * 8));
        }

        bishop_blocker_masks[square] = mask;
    }
}

/**
 * @brief Determines the possible moves for a rook given a blocker configuration.
 * Iterates through each direction until it reaches a blocker or the edge of the board.
 * 
 * @param sq The square index of the rook.
 * @param blocker The blocker configuration.
 * @return The possible moves for the rook.
 */
ULL determine_possible_rook_moves(uint8_t sq, ULL blocker)
{
    ULL mask = 0ULL;
    uint8_t square_rank = sq/8; 
    uint8_t square_file = sq%8;
    int rank, file;
    uint8_t possible_square;

    // north direction
    for (rank = square_rank+1; rank <= 7; rank++) {
    possible_square = square_file + rank*8;
    mask |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    // south direction
    for (rank = square_rank-1; rank >= 0; rank--) {
    possible_square = square_file + rank*8;
    mask |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    // east direction
    for (file = square_file+1; file <= 7; file++) {
    possible_square = file + square_rank*8;
    mask |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    // west direction
    for (file = square_file-1; file >= 0; file--) {
    possible_square = file + square_rank*8;
    mask |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    return mask;
}

ULL determine_possible_bishop_moves(uint8_t sq, ULL blocker)
{
    ULL mask = 0ULL;
    uint8_t square_rank = sq / 8;
    uint8_t square_file = sq % 8;
    int rank, file;
    uint8_t possible_square;

    // north-east direction
    for (rank = square_rank + 1, file = square_file + 1; rank <= 7 && file <= 7; rank++, file++)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square)) break;
    }
    // north-west direction
    for (rank = square_rank + 1, file = square_file - 1; rank <= 7 && file >= 0; rank++, file--)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square)) break;
    }
    // south-east direction
    for (rank = square_rank - 1, file = square_file + 1; rank >= 0 && file <= 7; rank--, file++)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square)) break;
    }
    // south-west direction
    for (rank = square_rank - 1, file = square_file - 1; rank >= 0 && file >= 0; rank--, file--)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square)) break;
    }

    return mask;
}



/**
 * @brief Generates all possible blockers and their corresponding magic numbers.
 * 
 * FOR EVERY SQUARE ON THE _BOARD (although the function is called individually for each square):
 * 
 * A random magic number is generated.
 * 
 * The number of permutations of blockers is found by counting the number of bits 
 * in the blocker mask relating to that square.
 * For each permutation, the possible moves for the rook wih that blocker configuration 
 * are determined using the determine_possible_rook_moves function.
 * 
 * The hash index for that permutation is then found,
 * using the formula: index = (blocker * magic_number) >> (64 - bits)
 * 
 * The value at the index in the lookup table is then checked, and if there is no collision,
 * the possible moves are stored in the lookup table at that index.
 * No collision means that the value at the index is either 0 or the same as the possible moves.
 * 
 * If there is a collision, the function returns false and the entire process is repeated for that square.
 * Once the function returns true, the magic number is stored in the magic number array,
 * and a new square is passed into the function.
 * 
 * 
 * The function has three parts:
 * - selection of appropriate tables depending on the piece type
 * - generation of appropriate blocker permutation
 * - testing and assignment of magic number
 * 
 * @param square The square index of the piece.
 * @return true if the magic number was generated successfully, false otherwise.
 */
bool generate_possible_blockers_and_magic_numbers(uint8_t square, bool rook)
{
    ULL* magic_numbers_array;
    ULL* blocker_masks;
    if (rook)
    {
        magic_numbers_array = actual_rook_magic_numbers;
        blocker_masks = rook_blocker_masks;
    }
    else
    {
        magic_numbers_array = actual_bishop_magic_numbers;
        blocker_masks = bishop_blocker_masks;
    }
    
    ULL mask = blocker_masks[square];
    int bits = __builtin_popcountll(mask);
    int permutations = 1 << bits;
    ULL magic_number;

    if (KNOWN_MAGIC_NUMBERS) 
    {
        magic_number = magic_numbers_array[square];
    } else 
    {
        magic_number = random_ULL_fewbits();
    }


    // all the permutations are simply numbers from 0 to 2^bits
    // this code sets the blocker mask bits to that permutation value
    for (int i = 0; i < permutations; i++)
    {
        ULL blocker = 0;
        int bit_index = 0;
        // iterates through all the squares on the board
        for (int j = 0; j < 64; j++)
        {
            // if the square on the board is a relevant blocker
            if (mask & (1ULL << j))
            {
                // then set the bit if the bit_index-th bit of the permutation value is set
                if (i & (1 << bit_index))
                {
                    blocker |= (1ULL << j);
                }
                bit_index++;
            }
        }


        if (rook)
        {
             // determine possible moves for that permutation
            ULL possible_moves = determine_possible_rook_moves(square, blocker);
            ULL index = (blocker * magic_number) >> (64 - bits);
            ULL current = rook_attack_lookup_table[square][index];


            if (KNOWN_MAGIC_NUMBERS) 
            {
                rook_attack_lookup_table[square][index] = possible_moves;
                array_for_rook_magic_numbers[square] = magic_number;
            } else 
            {
                // if there is no collision, store the possible moves in the lookup table
                if (current == possible_moves) {
                    continue;
                } else if (!current) {
                    rook_attack_lookup_table[square][index] = possible_moves;
                } else {
                    // counter++;
                    return 0;
                }
            }
        } else {

            ULL possible_moves = determine_possible_bishop_moves(square, blocker);
            ULL index = (blocker * magic_number) >> (64 - bits);
            ULL current = bishop_attack_lookup_table[square][index];

            if (KNOWN_MAGIC_NUMBERS) 
            {
                bishop_attack_lookup_table[square][index] = possible_moves;
                array_for_bishop_random_numbers[square] = magic_number;
            } else 
            {
                if (current == possible_moves) {
                    continue;
                } else if (!current) {
                    bishop_attack_lookup_table[square][index] = possible_moves;
                } else {
                    // counter++;
                    return 0;
                }
            }
        }
    }



    if (!KNOWN_MAGIC_NUMBERS) {
        // printf("Collision detected %llu times\n", counter);
        if (rook) {
            array_for_rook_magic_numbers[square] = magic_number;
        } else {
            array_for_bishop_random_numbers[square] = magic_number;
        }
    }
    return 1;
}


void rook_attack_generator(void)
{
    generate_rook_blocker_masks();
    for (int square = 0; square < 64; square++)
    {
        while (!generate_possible_blockers_and_magic_numbers(square, true))
        {
            for (int i = 0; i < 4096; i++)
            {
                rook_attack_lookup_table[square][i] = 0;
            }
        }
    }
}

void bishop_attack_generator(void)
{
    generate_bishop_blocker_masks();
    for (int square = 0; square < 64; square++)
    {
        while (!generate_possible_blockers_and_magic_numbers(square, false))
        {
            for (int i = 0; i < 4096; i++)
            {
                bishop_attack_lookup_table[square][i] = 0;
            }
        }
    }
}

void generate_lookup_tables()
{
    pawn_attack_generator();
    knight_attack_generator();
    king_attack_generator();
    rook_attack_generator();
    bishop_attack_generator();
}
