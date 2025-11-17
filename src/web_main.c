/**
 * @file web_main.c
 * @author Philip Brand
 * @brief Main file for building the web interface version of the chess engine
 * @date 2025-11-16
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"
#include "./movefinding/memory.h"
#include "./search/search.h"
#include "./search/hash_tables.h"
#include "./gui/gui.h"
#include "./interface/ui.h"
#include "./interface/movedisplay.h"

#define new "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
// #define new "3krr2/8/8/8/8/8/3K4/8 w - - 20 21"
// #define new "r2q1rk1/1pp1nppp/8/p1b1p3/2PpP1n1/PP1P2N1/2Q2PPP/RNB2RK1 w - - 0 13"
// #define new "8/7K/5k2/8/8/6q1/8/8 w - - 0 1"

static bool playing_as_white = false; // Default perspective for printing the board
static Position_t old_position; // Current position of the game
static Position_t new_position; // Position after the last move

static char previous_fen_string[FEN_LENGTH] = {0};
static char fen_string[FEN_LENGTH] = {0};

bool play_game(Position_t* position);
bool update_game(void);
void init(void);
void cli_game_loop(void* arg);

int main(void)
{
    init();
    cli_game_loop(NULL);
    return 0;
}

// Thread function for CLI game loop
void cli_game_loop(void* arg)
{
    (void)arg; // Unused parameter
    set_time(1);
    set_colour(&playing_as_white);
    start_clock(); // Start the clock for the first player

    while (1) { if (!play_game(&old_position)) 
        { break; /* Exit the game loop if game is over */ } }

    custom_memory_deinit();
    hash_table_free();
}

bool play_game(Position_t* position)
{
    // if (!update_game()) { return 0; /* Exit if the game is over */ }

    static bool first_move = true;

    if (first_move && !playing_as_white) {
        // If the user is playing as black, the engine makes the first move
        find_best_move(position, &new_position, 30, get_next_move_search_time());
        if (!update_game()) { return 0; /* Exit if the game is over */ }
        first_move = false;
    }

    // user move
    bool success = 0;
    while (!success) {
        bool success1 = read_fen_from_stdin(fen_string);
        pad_fen_to_full_length(fen_string);
        fen_to_board(fen_string, &new_position);
        char move_notation[MOVE_NOTATION_LENGTH] = {0};
        get_move_notation(position, &new_position, move_notation);
        bool success2 = make_move_from_san(position, &new_position, move_notation);
        success = success1 && success2;
        if (!success) 
            {printf("Invalid move. Please try again.\n"); fflush(stdout); }
    }

    if (!update_game()) { return 0; /* Exit if the game is over */ }

    // engine move
    find_best_move(position, &new_position, 20, get_next_move_search_time());
    if (!update_game()) { return 0; /* Exit if the game is over */ }
    board_to_fen(&old_position, fen_string);
    printf("%s\n", fen_string);
    fflush(stdout);

    return 1;
}

bool update_game(void)
{
    old_position = new_position;
    insert_past_move_entry(&old_position); // Insert the new position into the past move list
    switch_time_decrement(); // Switch the time decrement between user and engine
    update_time_display(); // Update the time display for both players
    if (is_game_ended(&old_position)) { return 0; }
    return 1; // Game continues
}

void init(void)
{
    strncpy(previous_fen_string, new, FEN_LENGTH);
    fen_to_board(previous_fen_string, &old_position);
    custom_memory_init();
    move_finder_init();
    zobrist_key_init();
    hash_table_init();
    ui_init();
    fen_to_board(new, &old_position);
    insert_past_move_entry(&old_position);
}

