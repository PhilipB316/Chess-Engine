// main.c

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <SDL2/SDL.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/board.h"
#include "./movefinding/memory.h"
#include "./search/search.h"
#include "./ui/ui.h"
#include "./ui/gui.h"

static bool playing_as_white = false; // Default perspective for printing the board

static Position_t position; // Current position of the game
static Position_t move_position; // Position after the last move


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
    init(); // Initialize memory, move finder, and UI

    pthread_t cli_thread, sdl_thread;

    // Start CLI game loop thread
    pthread_create(&cli_thread, NULL, cli_game_loop, NULL);

    // Start SDL GUI loop thread
    pthread_create(&sdl_thread, NULL, sdl_gui_loop, NULL);

    // Wait for both threads to finish
    pthread_join(cli_thread, NULL);
    pthread_join(sdl_thread, NULL);

    return 0;
}

// Thread function for CLI game loop
void* cli_game_loop(void* arg)
{
    char new[FEN_LENGTH] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    print_name();
    print_welcome_message();
    set_time();
    set_colour(&playing_as_white);
    fen_to_board(new, &position);
    printf("\n");
    print_position(&position);
    start_clock(); // Start the clock for the first player

    while (1) {
        if (!play_game(&position)) { break; /* Exit the game loop if game is over */ }
    }

    check_memory_leak();
    custom_memory_deinit();
    return NULL;
}


bool play_game(Position_t* position)
{
    static bool first_move = true;

    if (first_move && !playing_as_white) {
        // If the user is playing as black, the engine makes the first move
        find_best_move(position, &move_position, 20, get_next_move_search_time());
        if (!update_game()) { return 0; /* Exit if the game is over */ }
        first_move = false;
    }

    // user move
    make_move_from_cli(position, &move_position);
    if (!update_game()) { return 0; /* Exit if the game is over */ }

    // engine move
    find_best_move(position, &move_position, 20, get_next_move_search_time());
    if (!update_game()) { return 0; /* Exit if the game is over */ }

    return 1;
}

bool update_game(void)
{
    clear_output_screen(15);
    position = move_position;
    print_position(&position);
    switch_time_decrement(); // Switch the time decrement between user and engine
    update_time_display(); // Update the time display for both players
    if (is_game_ended(&position)) { return 0; }
    return 1; // Game continues
}

void init(void)
{
    custom_memory_init();
    move_finder_init();
    ui_init();
}

