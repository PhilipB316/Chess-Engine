/**
* @file ui.h
* @brief User interface for chess engine
* @author Philip Brand
* @date 2025-06-21
*/

#ifndef UI_H
#define UI_H

#include <stdint.h>

#include "../movefinding/board.h"

#define MOVE_LENGTH 10

#define COLOUR_BOLD "\e[1m"
#define COLOUR_RESET "\e[m"

/**
 * @brief Initializes the user interface.
 * 
 * This function sets up the user interface for the chess engine, preparing it for interaction.
 */
void ui_init(void);

/**
 * @brief Prints the board position in ascii format.
 * 
 * Includes ranks and files labels for better readability.
 * 
 * @param position The position to be printed.
 */
void print_position(Position_t* position);

/**
 * @brief Makes a move from command line input.
 * 
 * Prompts the user for a move in standard chess notation and applies it to the given position.
 * If the user inputs "exit", the function returns 0, otherwise it returns 1 after making the move.
 * 
 * @param position Pointer to the current position.
 * @param move_position Pointer to the position where the move will be applied.
 * @return 1 if a move was made, 0 if the user chose to exit.
 */
uint8_t make_move_from_cli(Position_t *position, Position_t *move_position);

/**
 * @brief Prints the name and version of the chess engine.
 * 
 * Displays the name, author, and version information of the chess engine.
 */
void print_name(void);

/**
 * @brief Prints a welcome message to the user.
 * 
 * Displays a welcome message and instructions for the user when they start the chess engine.
 */
void print_welcome_message(void);

#endif // UI_H
