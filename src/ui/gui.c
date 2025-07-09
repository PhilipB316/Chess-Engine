// File: gui.c

#include <SDL2/SDL.h>
#include "gui.h"

void render_chess_board(SDL_Renderer* renderer)
{
    SDL_Color light = {240, 217, 181, 255};
    SDL_Color dark = {181, 136, 99, 255};

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            SDL_Rect square = {x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
            if ((x + y) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, light.r, light.g, light.b, light.a);
            } else {
                SDL_SetRenderDrawColor(renderer, dark.r, dark.g, dark.b, dark.a);
            }
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

// Thread function for SDL GUI loop
void* sdl_gui_loop(void* arg) 
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Chess Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_chess_board(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return NULL;
}

