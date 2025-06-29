// main.c

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"
#include "./movefinding/memory.h"
#include "./search/search.h"
#include "./ui/ui.h"

static uint16_t max_search_time = 5; // Default search time in seconds

/**
    * TODO:
    * - display principal variation
    * - implement multi-threading
    * - improve position evaluation
    * - implement quiescence search
*/

uint8_t play_game(Position_t* position);

int main(void)
{
    // char fen1[FEN_LENGTH] = "r2qkb1r/p1pp1p1p/bpn2n1B/3Pp1p1/3p4/2N1PNPB/PPPQ1P1P/R3K2R w KQkq e6 0 10";
    // char fen2[FEN_LENGTH] = "r3kb1r/pp1npppp/2p2n2/q2p2B1/2PP2b1/2N2P2/PPQ1P1PP/R3KBNR w KQkq - 3 15";
    // char fen3[FEN_LENGTH] = "rnb1kbnr/pppppppp/8/8/8/4P3/PPPP1PPP/R3K2R w KQkq - 0 1";
    // char fen4[FEN_LENGTH] = "4k3/2p2p2/1p4r1/6P1/7P/1P6/3K4/8 w - - 0 1";
    // char fen5[FEN_LENGTH] = "k4q2/6P1/8/8/8/8/8/7K w - - 0 1";
    // char fen6[FEN_LENGTH] = "1k6/8/2p5/3Pp3/8/8/8/2K5 w - e6 0 2";
    // char fen7[FEN_LENGTH] = "6k1/8/4r3/6r1/6b1/5rr1/8/R3K2R w - - 0 1";
    // char fen9[FEN_LENGTH] = "r1bn1r2/p4Npk/1p5p/1qp2p1Q/3p4/P2P3P/BPP2PP1/4RRK1 w - - 4 21";
    // char fen10[FEN_LENGTH] ="3r2k1/b4bp1/2pRn2p/1p2N3/1P6/1B5P/5PP1/6K1 w - - 3 30";
    // char fen11[FEN_LENGTH] = "r2q3r/1p2kp2/4b2p/pB2Q3/4Nn2/2P4P/P1K2P2/R6R w - - 1 24";
    // char fen12[FEN_LENGTH] = "rnb1k1nr/ppp1pppp/3p4/2b5/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // char fen13[FEN_LENGTH] = "r1bqkb1r/pppp1pp1/B1n2n1p/4p3/4P3/2N2N1P/PPPP1PP1/R1BQK2R b Qq - 5 7";
    // char fen[FEN_LENGTH] = "1k5R/6R1/8/8/8/3K4/8/8 b - - 0 1";
    // char fen14[FEN_LENGTH] = "1k6/6R1/7R/8/8/3K4/8/8 b - - 0 1";
    // char fen15[FEN_LENGTH] = "rnbqkbnr/pppppppp/8/8/8/2P5/PP1PPPPP/R3K2R w KQkq - 0 1";

    char new[FEN_LENGTH] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Position_t position;

    custom_memory_init();
    move_finder_init();
    ui_init();
    print_name();
    print_welcome_message();
    set_search_time(&max_search_time);
    fen_to_board(new, &position);
    print_position(&position);

    while (1)
    {
        if (!play_game(&position)) { break; /* Exit the game loop if no valid move is made */ }
    }
 
    check_memory_leak();
    custom_memory_deinit();
}

uint8_t play_game(Position_t* position)
{
    // user move
    Position_t move_position;
    make_move_from_cli(position, &move_position);
    *position = move_position;
    print_position(position);
    if (is_game_ended(position)) { return 0; }

    // engine move
    find_best_move(position, &move_position, 20, max_search_time);
    print_stats();
    *position = move_position;
    print_position(position);
    if (is_game_ended(position)) { return 0; }

    return 1;
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


/**
 * e4, Qf3, Nh3, Bd3, 0-0, Nc3, Qe3, fxe3, cxd3, b3, d4, Nf2, exd4, Re1, a3, axb4
*/
