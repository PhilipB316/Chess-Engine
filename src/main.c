#include <stdio.h>
#include <stdint.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"
#include "./search/search.h"

/**
    * TODO: 
    * - shift memory allocation to locally managed memory
    * - implement alpha-beta pruning
    * - implement move ordering
    * - improve position evaluation
*/

int main(void)
{
    printf("Hello, World!\n");
    move_finder_init();

    // char fen1[100] = "r2qkb1r/p1pp1p1p/bpn2n1B/3Pp1p1/8/2N1PNPB/PPPQ1P1P/R3K2R w KQkq e6 0 10";
    // char fen2[100] = "r3kb1r/pp1npppp/2p2n2/q2p2B1/2PP2b1/2N2P2/PPQ1P1PP/R3KBNR w KQkq - 3 7";
    // char fen3[100] = "rnb1kbnr/pppppppp/8/8/8/4P3/PPPP1PPP/R3K2R w KQkq - 0 1";
    // char fen4[100] = "2k5/8/8/4Pp2/8/8/8/2K5 w - f6 0 2";
    // char fen5[100] = "k4q2/6P1/8/8/8/8/8/7K w - - 0 1";
    // char fen6[100] = "1k6/8/2p5/3Pp3/8/8/8/2K5 w - e6 0 2";
    // char fen7[100] = "6k1/8/4r3/6r1/6b1/5rr1/8/R3K2R w - - 0 1";
    char fen8[100] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // char fen9[100] = "r1bn1r2/p4Npk/1p5p/1qp2p1Q/3p4/P2P3P/BPP2PP1/4RRK1 w - - 4 21";
    // char fen10[100] ="3r2k1/b4bp1/2pRn2p/1p2N3/1P6/1B5P/5PP1/6K1 w - - 3 30";
    // char fen11[100] = "r2q3r/1p2kp2/4b2p/pB2Q3/4Nn2/2P4P/P1K2P2/R6R w - - 1 24";
    // char fen[100] = "r3k2r/ppp1pp1p/2q2np1/bb6/1n1pP3/1Q5B/PPPP1PPP/RNB1K1NR b KQkq e3 0 2";
    Position_t position, best_move;
    fen_to_board(fen8, &position);
    print_position(&position);
    find_best_move(&position, &best_move, 6);
    printf("Number of new positions generated: %lu\n", get_num_new_positions());
    printf("Best move:\n");
    print_position(&best_move);
}

