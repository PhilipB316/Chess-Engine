/**
* @file gui.h
* @brief Graphical user interface for chess engine
* @author Philip Brand
* @date 2025-07-09
*/

#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>

#include "../movefinding/board.h"

#define BOARD_WIDTH 640
#define BOARD_HEIGHT 640
#define BOARD_SIZE 8
#define RIGHT_BORDER_WIDTH 200
#define SQUARE_SIZE (BOARD_WIDTH / BOARD_SIZE)

#define PADDING SQUARE_SIZE / 8

typedef struct {
    Position_t* position;
    bool* playing_as_white;
} GUI_Args_t;

/**
 * @brief Thread function for the SDL GUI loop
 *
 * @param arg Pointer to GUI_Args_t containing position and playing perspective
 * @return Pointer to the result (NULL in this case)
 */
void* sdl_gui_loop(void* arg);

#endif // GUI_H
