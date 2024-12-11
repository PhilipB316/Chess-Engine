#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#include "movefinder.h"
#include "move_lookuptables.h"


int main(void)
{
    Move_t move_list[300] = {0};
    size_t num_moves = 0;
    printf("Hello, World!\n");

    // char fen[100] = "7k/8/8/2Pp4/8/8/7K/8 w - d6 0 2";
    char fen[100] = "r3kb1r/pp1npppp/2p2n2/q2p2B1/2PP2b1/2N2P2/PPQ1P1PP/R3KBNR w KQkq - 3 7";
    // char fen[100] = "6k1/8/4r3/6r1/6b1/5rr1/8/R3K2R w - - 0 1";
    Position_t position = fen_to_board(fen);
    print_bitboard(position.black_pieces.kings);
    printf("White to move: %d\n", position.white_to_move);

    generate_lookup_tables();
    clock_t start_time = clock();
    for (int i = 0; i < 10000000; i++)
    {
        PiecesOneColour_t active_pieces, opponent_pieces;
        if (position.white_to_move) {
            active_pieces = position.white_pieces;
            opponent_pieces = position.black_pieces;
        } else {
            active_pieces = position.black_pieces;
            opponent_pieces = position.white_pieces;
        }
        num_moves = 0;
        move_finder(move_list, &num_moves, &position, &active_pieces, &opponent_pieces);
        // printf("Number of times queen_move_finder was called: %d\n", i);
    }
    clock_t end_time = clock();
    
    print_moves(move_list, num_moves);
    printf("Time taken: %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
}
