#include <stddef.h>
#include <stdio.h>
#include "position.h"

int main(void)
{
    char fen[100] = "rnbqkbnr/ppppp1pp/7p/8/8/3P2P1/PPP1PP1P/RNBQKBNR w KQkq - 0 1";
    Position_t position = fen_to_board(fen);
    position.is_white_turn = true;
    print_board(position.board);

    // for (uint64_t i = 0; i < 100000000; i++)
    // {
    //     determine_possible_moves(&position);
    //     if (i % 10000 == 0) {
    //         printf("Iteration: %lu\n", i);
    //     }
    // }
    determine_possible_moves(&position);
    print_moves(&position);
}