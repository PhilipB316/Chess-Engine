// File ui.c

#include "ui.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "movedisplay.h"
#include "../search/evaluate.h"
#include "../movefinding/movefinder.h"

static bool white_perspective = true; // Default perspective for printing the board

void ui_init(void)
{
    printf(COLOUR_BOLD "\n" COLOUR_RESET);
}

bool is_game_ended(Position_t *position)
{
    switch (determine_king_status(position))
    {
        case CHECK:
            return false;
        case CHECKMATE:
            if (position->white_to_move) {
                printf("Black wins by checkmate! Game over.\n");
            } else {
                printf("White wins by checkmate! Game over.\n");
            }
            return true;
        case STALEMATE:
            printf("Stalemate! Game over.\n");
            return true;
        case BORING:
            return false;
        default:
            return false;
    }
}

void set_search_time(uint16_t* max_search_time)
{
    printf("Please enter the maximum engine search time per move (in seconds): ");
    if (scanf("%hu", max_search_time) != 1 || *max_search_time == 0) {
        fprintf(stderr, "Invalid input. ");
        // Clear the input buffer to remove any leftover characters
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        set_search_time(max_search_time); // Retry if input is invalid
    } else {
        printf("Maximum search time set to %hu seconds.\n\n", *max_search_time);
        // Clear the input buffer to remove any leftover characters
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
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
}

int make_move_from_notation(char *move_notation, Position_t *source, Position_t *destination) 
{
    MoveType_t move_type;
    uint8_t to_square = 0;
    char disambiguation[2] = "\0";
    bool is_capture = false;
    ULL special_flags = 0;

    if (!parse_move_notation(move_notation, &move_type, &to_square, disambiguation, &is_capture, source, &special_flags)) {
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
        printf("This move is check: %s, please try again.\n", move_notation);
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
    printf("|                    24-06-2025                   |\n");
    printf("|                   Version  1.1                  | \n");
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

