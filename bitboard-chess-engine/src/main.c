#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"

int main(void)
{
    printf("Hello, World!\n");
    move_finder_init();

    char fen[100] = "r2qkb1r/p1pp1p1p/bpn2n1B/3Pp1p1/8/2N1PNPB/PPPQ1P1P/R3K2R w KQkq e6 0 10";
    // char fen[100] = "r3kb1r/pp1npppp/2p2n2/q2p2B1/2PP2b1/2N2P2/PPQ1P1PP/R3KBNR w KQkq - 3 7";
    // char fen[100] = "6k1/8/4r3/6r1/6b1/5rr1/8/R3K2R w - - 0 1";

    Position_t position;
    fen_to_board(fen, &position);
    print_position(position);
    printf("White to move: %d\n", position.white_to_move);

    clock_t start_time = clock();
    move_finder(&position);
    // time something
    clock_t end_time = clock();
    printf("Time taken: %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    printf("-----finished------\n");
}
