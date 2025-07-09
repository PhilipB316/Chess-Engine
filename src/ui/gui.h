/**
* @file gui.h
* @brief Graphical user interface for chess engine
* @author Philip Brand
* @date 2025-07-09
*/

#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
#define BOARD_SIZE 8
#define SQUARE_SIZE (WINDOW_WIDTH / BOARD_SIZE)

#define PADDING SQUARE_SIZE / 8

static char* piece_files[12] = {
    "../assets/pieces/w_pawn_png_shadow_1024px.png",
    "../assets/pieces/w_knight_png_shadow_1024px.png",
    "../assets/pieces/w_bishop_png_shadow_1024px.png",
    "../assets/pieces/w_rook_png_shadow_1024px.png",
    "../assets/pieces/w_queen_png_shadow_1024px.png",
    "../assets/pieces/w_king_png_shadow_1024px.png",
    "../assets/pieces/b_pawn_png_shadow_1024px.png",
    "../assets/pieces/b_knight_png_shadow_1024px.png",
    "../assets/pieces/b_bishop_png_shadow_1024px.png",
    "../assets/pieces/b_rook_png_shadow_1024px.png",
    "../assets/pieces/b_queen_png_shadow_1024px.png",
    "../assets/pieces/b_king_png_shadow_1024px.png"
};

/**
 * @brief Thread function for the SDL GUI loop
 *
 * @param arg Pointer to the current position structure
 * @return Pointer to the result (NULL in this case)
 */
void* sdl_gui_loop(void* arg);

#endif // GUI_H
