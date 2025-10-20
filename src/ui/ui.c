// File ui.c

#include "ui.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "movedisplay.h"
#include "../search/evaluate.h"
#include "../movefinding/movefinder.h"

#define MILLISECONDS_IN_SECOND 1000

static bool white_perspective = true; // Default perspective for printing the board

static uint16_t max_engine_search_time = 5;
static uint16_t user_time_increment = 0;
static uint16_t game_length_per_side = 0;

static uint16_t half_move_count = 0;

static LL user_time_remaining = 0;
static LL engine_time_remaining = 0;
static LL current_player_start_time = 0;
static bool playing_with_clock = false; // Flag to indicate if the game is played with a clock

static bool engine_thinking = false;

LL get_monotonic_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

void ui_init(void)
{
    printf(COLOUR_BOLD "\n" COLOUR_RESET);
}

void clear_input_buffer(void)
{
    // Clear the input buffer to remove any leftover characters
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void clear_output_screen(uint8_t x)
{
    for (int i = 0; i < x; i++) {
        printf("\033[1A");
        printf("\033[2K");
    }
}

void start_clock(void)
{
    current_player_start_time = get_monotonic_ms();
}

LL get_next_move_search_time(void)
{
    if (!playing_with_clock) { return max_engine_search_time;}
    else { 
        if (half_move_count < 2 ) {
            return max_engine_search_time * MILLISECONDS_IN_SECOND / 60;
        } else if (half_move_count < 10) {
            return engine_time_remaining / 60;
        } else if (half_move_count < 50) {
            return engine_time_remaining / 30;
        } else {
            return engine_time_remaining / 20;
        }
    }
}

void switch_time_decrement(void)
{
    half_move_count++;
    if (engine_thinking) {
        engine_thinking = false;
        engine_time_remaining -= (get_monotonic_ms() - current_player_start_time);
    } else {
        engine_thinking = true;
        user_time_remaining -= (get_monotonic_ms() - current_player_start_time);
        user_time_remaining += user_time_increment * MILLISECONDS_IN_SECOND;
    }
    current_player_start_time = get_monotonic_ms(); // Reset the start time for the next player
}

void update_time_display(void)
{
    if (!playing_with_clock) { return; }
    if (user_time_remaining < 0) { user_time_remaining = 0; }
    if (engine_time_remaining < 0) { engine_time_remaining = 0; }
    // printf("\033[2A");
    printf("User time remaining: %.1f seconds\n", (float)user_time_remaining / MILLISECONDS_IN_SECOND);
    printf("Engine time remaining: %.1f seconds\n", (float)engine_time_remaining / MILLISECONDS_IN_SECOND);
}

bool is_game_ended(Position_t *position)
{
    if (playing_with_clock) {
        if (user_time_remaining <= 0) {
            printf("User time is up! Engine wins by time out.\n");
            return true;
        } else if (engine_time_remaining <= 0) {
            printf("Engine time is up! User wins by time out.\n");
            return true;
        }
    }
    switch (determine_king_status(position))
    {
        case CHECK:
            return false;
        case CHECKMATE:
            if (position->white_to_move) { printf("Black wins by checkmate! Game over.\n"); } 
            else { printf("White wins by checkmate! Game over.\n"); }
            return true;
        case STALEMATE:
            printf("Stalemate! Game over.\n");
            return true;
        case THREEFOLD_REPETITION:
            printf("Threefold repetition! Game drawn.\n");
            return true;
        case BORING:
            return false;
        default:
            return false;
    }
}

void set_search_time(void)
{
    printf("\nPlease enter the maximum engine search time per move (in seconds)");
    printf("\n(Recommended: 1-5 seconds): ");
    if (scanf("%hu", &max_engine_search_time) != 1 || max_engine_search_time == 0) {
        fprintf(stderr, "Invalid input. ");
        clear_input_buffer();
        set_search_time(); // Retry if input is invalid
    } else {
        max_engine_search_time *= MILLISECONDS_IN_SECOND; // Convert to milliseconds
        printf("Maximum search time set to %hu seconds.\n\n", max_engine_search_time / MILLISECONDS_IN_SECOND);
        clear_input_buffer();
    }
}

void set_time_control(void)
{
    // Prompt for game length per side
    printf("\nPlease enter the game length per side in seconds: ");
    if (scanf("%hu", &game_length_per_side) != 1 || game_length_per_side == 0) {
        fprintf(stderr, "Invalid input. ");
        clear_input_buffer();
    } else {
        printf("Game length per side set to %hu seconds.\n", game_length_per_side);
        user_time_remaining = game_length_per_side * MILLISECONDS_IN_SECOND; // Convert to clock ticks
        engine_time_remaining = game_length_per_side * MILLISECONDS_IN_SECOND; // Convert to clock ticks
        max_engine_search_time = game_length_per_side;
    }
    printf("\nPlease enter the user time increment between moves in seconds: ");
    // Prompt for user time increment
    if (scanf("%hu", &user_time_increment) != 1) {
        fprintf(stderr, "Invalid input. ");
        clear_input_buffer();
        set_time_control(); // Retry if input is invalid
    } else {
        printf("User time increment set to %hu seconds.\n\n", user_time_increment);
        clear_input_buffer();
    }
}

void set_time(void)
{
    printf("Do you want to set an engine time per move, or a match time?\n");
    printf("Enter 'e' for engine time per move, or 'm' for match time: ");
    printf("\n(Recommended: 'e'): ");
    char choice;
    if (scanf(" %c", &choice) != 1 || (choice != 'e' && choice != 'm')) {
        fprintf(stderr, "Invalid input. Please enter 'e' or 'm'.\n");
        clear_input_buffer();
        set_time(); // Retry if input is invalid
        return;
    }
    if (choice == 'e') {
        printf("You chose engine time per move.\n");
        playing_with_clock = false;
        set_search_time();
    } else {
        printf("You chose match time.\n");
        playing_with_clock = true;
        set_time_control();
    }
}

void set_colour(bool* playing_as_white)
{
    char input[10];
    printf("Do you want to play as white or black? (w/b): ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0; // Remove newline character
        if (input[0] == 'w' || input[0] == 'W') {
            *playing_as_white = true;
            printf("You are playing as white.\n");
        } else if (input[0] == 'b' || input[0] == 'B') {
            *playing_as_white = false;
            printf("You are playing as black.\n");
        } else {
            fprintf(stderr, "Invalid input. Please enter 'w' or 'b'.\n");
            set_colour(playing_as_white); // Retry if input is invalid
        }
    } else {
        fprintf(stderr, "Error reading input. Please try again.\n");
        set_colour(playing_as_white); // Retry if fgets fails
    }
    white_perspective = *playing_as_white; // Set the perspective based on the player's choice

    if (white_perspective) {
        engine_thinking = false;
    } else {
        engine_thinking = true; // If playing as black, the engine will think first
    }
}

int make_move_from_notation(char *move_notation, Position_t *source, Position_t *destination) 
{
    MoveType_t move_type;
    uint8_t to_square = 0;
    char disambiguation[2] = "\0";
    bool is_capture = false;
    ULL special_flags = 0;

    if (!parse_move_notation(move_notation,
                             &move_type,
                             &to_square,
                             disambiguation,
                             &is_capture,
                             source,
                             &special_flags)) {
        fprintf(stderr, "Invalid move notation: %s, please try again\n", move_notation);
        return 0; // Invalid move notation
    }
    ULL from_square_bitboard = determine_from_square_bitboard(source, move_type, to_square, disambiguation);
    if (from_square_bitboard == 0) {
        fprintf(stderr, "This move is illegal: %s, please try again.\n", move_notation);
        return 0; // No valid from square found
    }
    ULL to_square_bitboard = 1ULL << to_square;
    if (!make_notation_move(source, 
                            destination, 
                            move_type, 
                            to_square_bitboard, 
                            from_square_bitboard, 
                            special_flags)) {
        printf("Your king is not healthy with this move!! - please try again.\n");
        return 0; // Move could not be made
    }

    return 1; // Move successfully made
}

void make_move_from_cli(Position_t *position, Position_t *move_position)
{
    char move_notation[MOVE_LENGTH];
    bool finished = false;
    while (!finished) {
        printf("Enter move: ");
        if (fgets(move_notation, MOVE_LENGTH, stdin) == NULL) { }
        move_notation[strcspn(move_notation, "\n")] = 0; // Remove newline character
        finished = make_move_from_notation(move_notation, position, move_position);
    }
}

void print_name(void)
{
    printf("==================================================|\n");
    printf("|     _______            __  __ ------------------|   \n");
    printf("|    |__   __|          |  \\/  | -----------------|  \n");
    printf("|       | | ___  ___ ___| \\  / | __ ___  __ ------|     \n");
    printf("|       | |/ _ \\/ __/ __| |\\/| |/ _` \\ \\/ / ------|     \n");
    printf("|       | |  __/\\__ \\__ \\ |  | | (_| |>  < -------|     \n");
    printf("|       |_|\\___||___/___/_|  |_|\\__,_/_/\\_\\ ------|     \n");
    printf("|                                                 |\n");

    printf("|=================================================|\n");
    printf("|                   PHILIP BRAND                  |\n");
    printf("|                    09-07-2025                   |\n");
    printf("|                   Version  2.0                  | \n");
    printf("|==================================================\n");
    printf("\n");
}

void print_welcome_message(void)
{
    printf("Welcom to TessMax!\n");
    printf("When prompted, please enter your move.\n");
    printf("BE AWARE: Mistakes in your input can lead to unexpected behavior!\n");
    printf("To terminate the game, press ctrl + C.\n\n");
}

void print_position(Position_t* position)
{
    char* mailboxes[64] = {0};
    PiecesOneColour_t white_pieces = position->pieces[WHITE_INDEX];
    PiecesOneColour_t black_pieces = position->pieces[!WHITE_INDEX];

    for (uint8_t i = 0; i < 64; i++)
    {
        if (white_pieces.pawns & (1ULL << i)) { mailboxes[i] = "♟ "; }
        else if (white_pieces.knights & (1ULL << i)) { mailboxes[i] = "♞ "; }
        else if (white_pieces.bishops & (1ULL << i)) { mailboxes[i] = "♝ "; }
        else if (white_pieces.rooks & (1ULL << i)) { mailboxes[i] = "♜ "; }
        else if (white_pieces.queens & (1ULL << i)) { mailboxes[i] = "♛ "; }
        else if (white_pieces.kings & (1ULL << i)) { mailboxes[i] = "♚ "; }
        else if (black_pieces.pawns & (1ULL << i)) { mailboxes[i] = "♙ "; }
        else if (black_pieces.knights & (1ULL << i)) { mailboxes[i] = "♘ "; }
        else if (black_pieces.bishops & (1ULL << i)) { mailboxes[i] = "♗ "; }
        else if (black_pieces.rooks & (1ULL << i)) { mailboxes[i] = "♖ "; }
        else if (black_pieces.queens & (1ULL << i)) { mailboxes[i] = "♕ "; }
        else if (black_pieces.kings & (1ULL << i)) { mailboxes[i] = "♔ "; }
        else { mailboxes[i] = ". "; }
    }
    printf("\n");
    if (white_perspective) {
        for (uint8_t rank = 0; rank < 8; rank++) {
            printf(COLOUR_BOLD "\n%d   " COLOUR_RESET, 8 - rank);
            for (uint8_t file = 0; file < 8; file++) {
                uint8_t idx = rank * 8 + file;
                printf("%s ", mailboxes[idx]);
            }
        }
        printf("\n\n%c   a  b  c  d  e  f  g  h\n\n", position->white_to_move ? 'W' : 'B');
    } else {
        for (int rank = 7; rank >= 0; rank--) {
            printf(COLOUR_BOLD "\n%d   " COLOUR_RESET, 8 - rank);
            for (int file = 7; file >= 0; file--) {
                uint8_t idx = rank * 8 + file;
                printf("%s ", mailboxes[idx]);
            }
        }
        printf("\n\n%c   h  g  f  e  d  c  b  a\n\n", position->white_to_move ? 'W' : 'B');
    }
}

