/**
 * @file magic_numbers.c
 * @brief This file contains the implementation of the magic bitboard attack generation.
 * @author Philip Brand
 * @date 2024-11-27
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

#include "lookuptables.h"
#include "movefinder.h"

#define KNOWN_MAGIC_NUMBERS 1

// masks to determine relevant blockers for rooks and bishops
static ULL rook_blocker_masks[64];
static ULL bishop_blocker_masks[64];

// magic numbers for rooks and bishops
static ULL array_for_rook_magic_numbers[64];
static ULL bishop_magic_numbers[64];

// lookup tables for rook and bishop attacks
static ULL rook_attack_lookup_table[64][4096];
static ULL bishop_attack_lookup_table[64][512];


// (64 - bits) numbers of bits rook blockers can occupy 
uint8_t offset_RBits[64] = 
{
    52, 53, 53, 53, 53, 53, 53, 52,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    52, 53, 53, 53, 53, 53, 53, 52
};

const static ULL actual_rook_magic_numbers[64] = 
{
756607761056301088ULL,
4917965982829391872ULL,
9259436018514335872ULL,
252227969560612864ULL,
1873504042192277572ULL,
4683744712145502216ULL,
10736588108788171008ULL,
72060078754236160ULL,
140880296050688ULL,
36239972015016000ULL,
288793611754082336ULL,
10980338876383625296ULL,
281562084086016ULL,
7219973898715005952ULL,
288511855423520780ULL,
703743277449344ULL,
10430337286754283586ULL,
4616189892934762561ULL,
845524712292416ULL,
738733825827278848ULL,
81910317868647472ULL,
10133648984637568ULL,
27025995828578306ULL,
24807181388087505ULL,
4611826895502458880ULL,
290517918687560456ULL,
436853014294175744ULL,
3891391591680573472ULL,
8798240768384ULL,
54608346652868736ULL,
36037885170811138ULL,
4621327094725559297ULL,
18020034588704898ULL,
794920529348804608ULL,
38984971611803648ULL,
4611765185420464128ULL,
653444742557733889ULL,
9379309380144470164ULL,
4616304006254301322ULL,
5089704017996ULL,
2305983773545627684ULL,
1478940450466431008ULL,
282162185437249ULL,
36310615627333664ULL,
145241122350858244ULL,
563259460157452ULL,
77124178800672772ULL,
2533275872591874ULL,
306325047602086400ULL,
9876464420289249344ULL,
7206920489413936256ULL,
5440992801384890624ULL,
290490978518433920ULL,
4400196747392ULL,
9377629124748796928ULL,
4611734676179550720ULL,
72093878194930306ULL,
1299358895885811745ULL,
3603162595299696641ULL,
1424967491126529ULL,
72339077738795013ULL,
563518231807746ULL,
4505833060814860ULL,
4684308548237328530ULL
};



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
    ULL result = 0ULL;
    uint8_t square_rank = sq/8; 
    uint8_t square_file = sq%8;
    int rank, file;
    uint8_t possible_square;

    // north direction
    for (rank = square_rank+1; rank <= 7; rank++) {
    possible_square = square_file + rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    // south direction
    for (rank = square_rank-1; rank >= 0; rank--) {
    possible_square = square_file + rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    // east direction
    for (file = square_file+1; file <= 7; file++) {
    possible_square = file + square_rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    // west direction
    for (file = square_file-1; file >= 0; file--) {
    possible_square = file + square_rank*8;
    result |= (1ULL << possible_square);
    if (blocker & (1ULL << possible_square)) break;
    }
    return result;
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
 * @param square The square index of the piece.
 * @return true if the magic number was generated successfully, false otherwise.
 */
bool generate_possible_rook_blockers_and_magic_numbers(uint8_t square)
{
    ULL mask = rook_blocker_masks[square];
    int bits = __builtin_popcountll(mask);
    int permutations = 1 << bits;
    ULL magic_number;

    if (KNOWN_MAGIC_NUMBERS) 
    {
        magic_number = actual_rook_magic_numbers[square];
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
    }

    if (!KNOWN_MAGIC_NUMBERS) {
        // printf("Collision detected %llu times\n", counter);
        array_for_rook_magic_numbers[square] = magic_number;
    }
    return 1;
}




int main(void)
{
    generate_rook_blocker_masks();
    for (int j = 0; j < 64; j++)
    {
        printf("Square %d\n", j);
        while (!generate_possible_rook_blockers_and_magic_numbers(j))
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

        ULL index = (blockers * array_for_rook_magic_numbers[i]) >> offset_RBits[i];
        printf("Moves: \n");
        print_bitboard(rook_attack_lookup_table[i][index]);
        srand(index);
        printf("\n\n");

    }
}