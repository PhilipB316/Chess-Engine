// File ui.c

#include "ui.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "movedisplay.h"
#include "../search/evaluate.h"

void ui_init(void)
{
    // Initialize any necessary UI components here.
    // For now, we just print the name and welcome message.
    printf(COLOUR_BOLD "\n" COLOUR_RESET);
}

uint8_t make_move_from_cli(Position_t *position, Position_t *move_position)
{
    printf("Enter move, or 'exit' to quit: ");
    char move_notation[MOVE_LENGTH];
    if (fgets(move_notation, MOVE_LENGTH, stdin) == NULL) {
        // Handle error or end-of-file
    }
    move_notation[strcspn(move_notation, "\n")] = 0; // Remove newline character

    if (!(strcmp(move_notation, "exit") == 0)) {

        while (!make_move_from_notation(move_notation, position, move_position)) {
            printf("Invalid move notation. Please try again: ");
            if (fgets(move_notation, MOVE_LENGTH, stdin) == NULL) {
                // Handle error or end-of-file
            }
            move_notation[strcspn(move_notation, "\n")] = 0; // Remove newline character
        }
        return 1;
    }
    return 0;
}

void print_position(Position_t* position)
{
    printf("\n");
    char* mailboxes[64] = {0};
    PiecesOneColour_t white_pieces = position->pieces[WHITE_INDEX];
    PiecesOneColour_t black_pieces = position->pieces[!WHITE_INDEX];

    for (uint8_t i = 0; i < 64; i++)
    {
        if (white_pieces.pawns & (1ULL << i))
        {
            // mailboxes[i] = 'P';
            mailboxes[i] = "♟ ";
        }
        else if (white_pieces.knights & (1ULL << i))
        {
            // mailboxes[i] = 'N';
            mailboxes[i] = "♞ ";
        }
        else if (white_pieces.bishops & (1ULL << i))
        {
            // mailboxes[i] = 'B';
            mailboxes[i] = "♝ ";
        }
        else if (white_pieces.rooks & (1ULL << i))
        {
            // mailboxes[i] = 'R';
            mailboxes[i] = "♜ ";
        }
        else if (white_pieces.queens & (1ULL << i))
        {
            // mailboxes[i] = 'Q';
            mailboxes[i] = "♛ ";
        }
        else if (white_pieces.kings & (1ULL << i))
        {
            // mailboxes[i] = 'K';
            mailboxes[i] = "♚ ";
        }
        else if (black_pieces.pawns & (1ULL << i))
        {
            // mailboxes[i] = 'p';
            mailboxes[i] = "♙ ";
        }
        else if (black_pieces.knights & (1ULL << i))
        {
            // mailboxes[i] = 'n';
            mailboxes[i] = "♘ ";
        }
        else if (black_pieces.bishops & (1ULL << i))
        {
            // mailboxes[i] = 'b';
            mailboxes[i] = "♗ ";
        }
        else if (black_pieces.rooks & (1ULL << i))
        {
            // mailboxes[i] = 'r';
            mailboxes[i] = "♖ ";
        }
        else if (black_pieces.queens & (1ULL << i))
        {
            // mailboxes[i] = 'q';
            mailboxes[i] = "♕ ";
        }
        else if (black_pieces.kings & (1ULL << i))
        {
            // mailboxes[i] = 'k';
            mailboxes[i] = "♔ ";
        }
        else
        {
            mailboxes[i] = ". ";
        }
    }

    for (uint8_t i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
        {
            printf(COLOUR_BOLD "\n%d   " COLOUR_RESET, 8 - i / 8);
        }
        printf("%s ", mailboxes[i]);
    }
    printf("\n\n%d   a  b  c  d  e  f  g  h\n", position->white_to_move);

    printf("\n");
    printf("score: %ld\n", evaluate_position(position));
    printf("\n");
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
    printf("|                   Version  1.0                  | \n");
    printf("|==================================================\n");
    printf("\n");
}

void print_welcome_message(void)
{
    printf("Welcom to TessMax!\n");
    printf("When prompted, please enter your move.\n");
    printf("BE AWARE: Mistakes in your input can lead to unexpected behavior.\n");
}
