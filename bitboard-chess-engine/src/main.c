#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"
#include "./movefinding/lookuptables.h"

#define NUM_FENS 8

int main(void)
{
    printf("Hello, World!\n");
    move_finder_init();

     char fen1[100] = "r2qkb1r/p1pp1p1p/bpn2n1B/3Pp1p1/8/2N1PNPB/PPPQ1P1P/R3K2R w KQkq e6 0 10";
     char fen2[100] = "r3kb1r/pp1npppp/2p2n2/q2p2B1/2PP2b1/2N2P2/PPQ1P1PP/R3KBNR w KQkq - 3 7";
     char fen3[100] = "rnb1kbnr/pppppppp/8/8/8/4P3/PPPP1PPP/R3K2R w KQkq - 0 1";
     char fen4[100] = "2k5/8/8/4Pp2/8/8/8/2K5 w - f6 0 2";
     char fen5[100] = "1k3q2/6P1/8/8/8/8/8/2K5 w - - 0 1";
     char fen6[100] = "1k6/8/2p5/3Pp3/8/8/8/2K5 w - e6 0 2";
     char fen7[100] = "6k1/8/4r3/6r1/6b1/5rr1/8/R3K2R w - - 0 1";
     char fen8[100] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    char* random_fens[NUM_FENS] = {fen1, fen2, fen3, fen4, fen5, fen6, fen7, fen8};

    Position_t position;
    print_position(position);
    printf("White to move: %d\n", position.white_to_move);

    for (int j = 0; j < NUM_FENS; j++)
    {
    clock_t start_time = clock();
        fen_to_board(random_fens[j], &position);
        print_position(position);
        for (int i = 0; i < 1000000; i++)
        {
            // do nothing
            position.num_children = 0;
            move_finder(&position);
            // print_children_positions(&position);
            free_position_memory(&position);
        }
    clock_t end_time = clock();
    printf("---time-taken---\n");
    printf("%f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    printf("-----finished------\n");
    }
}
