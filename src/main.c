// main.c

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"
#include "./movefinding/memory.h"
#include "./movefinding/movedisplay.h"
#include "./search/search.h"

/**
    * TODO:
    * - thoroughly test move display
    * - display principal variation
    * - take move input from user
    * - implement multi-threading
    * - improve position evaluation
    * - implement quiescence search
*/

int main(void)
{
    custom_memory_init();
    move_finder_init();

    // char game_1[100] = "rnbqkbnr/ppp1pppp/8/3p4/8/4P3/PPPP1PPP/RNBQKBNR w KQkq d6 0 2";

    char fen1[100] = "r2qkb1r/p1pp1p1p/bpn2n1B/3Pp1p1/3p4/2N1PNPB/PPPQ1P1P/R3K2R w KQkq e6 0 10";
    // char fen2[100] = "r3kb1r/pp1npppp/2p2n2/q2p2B1/2PP2b1/2N2P2/PPQ1P1PP/R3KBNR w KQkq - 3 15";
    // char fen3[100] = "rnb1kbnr/pppppppp/8/8/8/4P3/PPPP1PPP/R3K2R w KQkq - 0 1";
    // char fen4[100] = "2k5/8/8/4Pp2/8/8/8/2K5 w - f6 0 2";
    // char fen5[100] = "k4q2/6P1/8/8/8/8/8/7K w - - 0 1";
    // char fen6[100] = "1k6/8/2p5/3Pp3/8/8/8/2K5 w - e6 0 2";
    // char fen7[100] = "6k1/8/4r3/6r1/6b1/5rr1/8/R3K2R w - - 0 1";
    char fen8[100] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // char fen9[100] = "r1bn1r2/p4Npk/1p5p/1qp2p1Q/3p4/P2P3P/BPP2PP1/4RRK1 w - - 4 21";
    // char fen10[100] ="3r2k1/b4bp1/2pRn2p/1p2N3/1P6/1B5P/5PP1/6K1 w - - 3 30";
    // char fen11[100] = "r2q3r/1p2kp2/4b2p/pB2Q3/4Nn2/2P4P/P1K2P2/R6R w - - 1 24";
    // char fen12[100] = "rnb1k1nr/ppp1pppp/3p4/2b5/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // char fen13[100] = "r1bqkb1r/pppp1pp1/B1n2n1p/4p3/4P3/2N2N1P/PPPP1PP1/R1BQK2R b Qq - 5 7";
    // char fen[100] = "r3k2r/ppp1pp1p/2q2np1/bb6/1n1pP3/1Q5B/PPPP1PPP/RNB1K1NR b KQkq e3 0 2";
    // char fen14[100] = "8/1k4P1/8/8/8/8/8/2K5 w - - 0 1";
    char fen15[100] = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2";

    Position_t position, best_move;
    fen_to_board(fen15, &position);
    print_position(&position);
    find_best_move(&position, &best_move, 1);
    printf("Best move found:\n");
    print_position(&best_move);

    char move_notation[20];
    determine_move_notation(&position, &best_move, move_notation);
    printf("Best move notation: %s\n", move_notation);

    strcpy(move_notation, "Qf3");
    printf("Making move from notation: %s\n", move_notation);
    Position_t move_position;
    make_move_from_notation(move_notation, &position, &move_position);
    print_position(&move_position);

    fen_to_board(fen8, &position);
    while (1) {
        printf("Enter move notation (or 'exit' to quit): ");
        fgets(move_notation, sizeof(move_notation), stdin);
        move_notation[strcspn(move_notation, "\n")] = 0; // Remove newline character

        if (strcmp(move_notation, "exit") == 0) {
            break;
        }
        make_move_from_notation(move_notation, &position, &move_position);
        position = move_position;
        print_position(&position);
        find_best_move(&position, &move_position, 5);
        position = move_position;
        print_position(&position);

    }


    // char* san_move = "Kxb1";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "e3";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "Be5";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "0-0-0";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "R1a8";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "Q1xb1";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "g8=Q";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "Qc6xb6";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "bxc8=R";
    // make_move_from_notation(san_move, &position, &best_move);
    // san_move = "bxc6";
    // make_move_from_notation(san_move, &position, &best_move);

    check_memory_leak();
    custom_memory_deinit();
}

