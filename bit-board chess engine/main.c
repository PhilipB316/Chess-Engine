#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#include "movefinder.h"
#include "lookuptables.h"


int main(void)
{
    printf("Hello, World!\n");

    generate_lookup_tables(
        pawn_attack_lookup_table, 
        knight_attack_lookup_table);

    char fen[100] = "rnb1kbnr/pp1p1ppp/4pq2/2p5/3P4/2NQ1N2/PPP1PPPP/R1B1KB1R b KQkq - 3 4";
    Position_t position = fen_to_board(fen);
    // print_bitboard(position.white_pawns);
    // print_bitboard(pawn_attack_lookup_table[1][50]);
    print_bitboard(knight_attack_lookup_table[44]);
}  