#include <stddef.h>
#include <stdio.h>
#include "position.h"

int main(void)
{
    char fen[100] = "rnb1kbnr/pp1p1ppp/4pq2/2p5/3P4/2NQ1N2/PPP1PPPP/R1B1KB1R b KQkq - 3 4";
    Position_t position = fen_to_board(fen);
    position.is_white_turn = false;
    print_board(position.board);

    for (uint64_t i = 0; i <= 1000000; i++)
    {
        determine_possible_moves(&position);
        if (i % 10000 == 0) {
            printf("Iteration: %lu\n", i);
        }
    }
    determine_possible_moves(&position);
    print_moves(&position);
}