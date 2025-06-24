// File ui.c

#include "ui.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "movedisplay.h"

uint8_t make_move_from_cli(Position_t *position, Position_t *move_position)
{
    printf("Enter move, or 'exit' to quit: ");
    char move_notation[MOVE_LENGTH];
    if (fgets(move_notation, MOVE_LENGTH, stdin) == NULL) {
        // Handle error or end-of-file
    }
    move_notation[strcspn(move_notation, "\n")] = 0; // Remove newline character

    if (!(strcmp(move_notation, "exit") == 0)) {
        make_move_from_notation(move_notation, position, move_position);
        return 1;
    }
    return 0;
}

void print_name(void)
{
    printf("\n");
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
