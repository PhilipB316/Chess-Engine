/**
 * @file local_main.c
 * @author Philip Brand
 * @brief Main file for local chess game between user and engine
 * @version 2.1.1
 * @date 2025-11-16
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <SDL2/SDL.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"
#include "./movefinding/memory.h"
#include "./search/search.h"
#include "./search/hash_tables.h"
#include "./gui/gui.h"
#include "./interface/ui.h"
#include "./gui/log.h"

static bool playing_as_white = false; // Default perspective for printing the board
#define new "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
// #define new "3krr2/8/8/8/8/8/3K4/8 w - - 20 21"
// #define new "r2q1rk1/1pp1nppp/8/p1b1p3/2PpP1n1/PP1P2N1/2Q2PPP/RNB2RK1 w - - 0 13"
// #define new "8/7K/5k2/8/8/6q1/8/8 w - - 0 1"

static Position_t position; // Current position of the game
static Position_t move_position; // Position after the last move

static GUI_Args_t gui_args; // Arguments for the GUI thread

/**
    * TODO:
    * - display principal variation
    * - implement multi-threading
    * - improve position evaluation
    * - implement quiescence search
*/

bool play_game(Position_t* position);
bool update_game(void);
void init(void);
void* cli_game_loop(void* arg);

int main(void)
{
    init();
    touch_log_file();

    pthread_t cli_thread, sdl_thread;
    pthread_create(&cli_thread, NULL, cli_game_loop, NULL);
    pthread_create(&sdl_thread, NULL, sdl_gui_loop, &gui_args);
    pthread_join(cli_thread, NULL);
    pthread_join(sdl_thread, NULL);
    return 0;
}

// Thread function for CLI game loop
void* cli_game_loop(void* arg)
{
    // int32_t evl = find_best_move(&position, &move_position, 6, 5000);
    // printf("Engine evaluation: %d\n", evl);
    // print_stats();
    // return NULL;

    (void)arg; // Unused parameter
    print_name();
    print_welcome_message();
    set_time(0);
    set_colour(&playing_as_white);
    write_log_pgn_header(playing_as_white);
    printf("\n");
    start_clock(); // Start the clock for the first player

    while (1) { if (!play_game(&position)) { break; /* Exit the game loop if game is over */ } }

    check_memory_leak();
    custom_memory_deinit();
    hash_table_free();
    return NULL;
}

bool play_game(Position_t* position)
{
    // if (!update_game()) { return 0; /* Exit if the game is over */ }

    static bool first_move = true;

    if (first_move && !playing_as_white) {
        // If the user is playing as black, the engine makes the first move
        find_best_move(position, &move_position, 30, get_next_move_search_time());
        if (!update_game()) { return 0; /* Exit if the game is over */ }
        first_move = false;
    }

    // user move
    make_move_from_cli(position, &move_position);
    if (!update_game()) { return 0; /* Exit if the game is over */ }

    // engine move
    find_best_move(position, &move_position, 20, get_next_move_search_time());
    // printf("Engine evaluation: %d\n", score);
    print_stats();
    if (!update_game()) { return 0; /* Exit if the game is over */ }

    return 1;
}

bool update_game(void)
{
    position = move_position;
    insert_past_move_entry(&position); // Insert the new position into the past move list
    switch_time_decrement(); // Switch the time decrement between user and engine
    update_time_display(); // Update the time display for both players
    if (is_game_ended(&position)) { return 0; }
    return 1; // Game continues
}

void init(void)
{
    gui_args.position = &position;
    gui_args.playing_as_white = &playing_as_white;
    custom_memory_init();
    move_finder_init();
    zobrist_key_init();
    hash_table_init();
    ui_init();
    fen_to_board(new, &position);
    insert_past_move_entry(&position);
}

