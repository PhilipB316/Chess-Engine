// File: gui.c

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "gui.h"
#include "../ui/ui.h"
#include "../movefinding/board.h"
#include "../search/evaluate.h"

char board[BOARD_SIZE][BOARD_SIZE] = {0};
SDL_Texture* piece_textures[12] = {NULL};

static bool* playing_as_white; // Default perspective for printing the board
static ULL from_bitboard, to_bitboard;
static ULL check_square_bitboard = 1;

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

void load_piece_textures(SDL_Renderer* renderer) {

    for (int i = 0; i < 12; i++) {
        SDL_Surface* surface = IMG_Load(piece_files[i]);
        if (surface == NULL) {
            printf("Warning: Could not load %s: %s\n", piece_files[i], IMG_GetError());
            piece_textures[i] = NULL;
        } else {
            piece_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }
}

int piece_char_to_index(char piece) {
    switch (piece) {
        case 'P': return 0; // White Pawn
        case 'N': return 1; // White Knight
        case 'B': return 2; // White Bishop
        case 'R': return 3; // White Rook
        case 'Q': return 4; // White Queen
        case 'K': return 5; // White King
        case 'p': return 6; // Black Pawn
        case 'n': return 7; // Black Knight
        case 'b': return 8; // Black Bishop
        case 'r': return 9; // Black Rook
        case 'q': return 10; // Black Queen
        case 'k': return 11; // Black King
        default: return -1; // Empty square or unknown piece
    }
}

void position_to_board_array(Position_t* position, char board[BOARD_SIZE][BOARD_SIZE]) 
{
    // clear the board array
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = '.';
        }
    }

    PiecesOneColour_t white_pieces = position->pieces[WHITE_INDEX];
    PiecesOneColour_t black_pieces = position->pieces[!WHITE_INDEX];

    for (uint8_t i = 0; i < 64; i++) {
        int rank = i / 8;
        int file = i % 8;
        int display_rank = *playing_as_white ? rank : (BOARD_SIZE - 1 - rank);
        int display_file = *playing_as_white ? file : (BOARD_SIZE - 1 - file);

        if (white_pieces.pawns & (1ULL << i)) { board[display_rank][display_file] = 'P'; }
        else if (white_pieces.knights & (1ULL << i)) { board[display_rank][display_file] = 'N'; }
        else if (white_pieces.bishops & (1ULL << i)) { board[display_rank][display_file] = 'B'; }
        else if (white_pieces.rooks & (1ULL << i)) { board[display_rank][display_file] = 'R'; }
        else if (white_pieces.queens & (1ULL << i)) { board[display_rank][display_file] = 'Q'; }
        else if (white_pieces.kings & (1ULL << i)) { board[display_rank][display_file] = 'K'; }
        else if (black_pieces.pawns & (1ULL << i)) { board[display_rank][display_file] = 'p'; }
        else if (black_pieces.knights & (1ULL << i)) { board[display_rank][display_file] = 'n'; }
        else if (black_pieces.bishops & (1ULL << i)) { board[display_rank][display_file] = 'b'; }
        else if (black_pieces.rooks & (1ULL << i)) { board[display_rank][display_file] = 'r'; }
        else if (black_pieces.queens & (1ULL << i)) { board[display_rank][display_file] = 'q'; }
        else if (black_pieces.kings & (1ULL << i)) { board[display_rank][display_file] = 'k'; }
    }
}

void render_chess_board(SDL_Renderer* renderer)
{

    SDL_Color light = {240, 217, 181, 255};        // Standard light squares
    SDL_Color dark = {181, 136, 99, 255};          // Standard dark squares
    SDL_Color to = {210, 180, 140, 255};    // Tan for dark "from"
    SDL_Color from = {147, 112, 219, 255};      // Medium slate blue for dark "to"
    SDL_Color check = {255, 0, 0, 255}; // Red for check square

    uint8_t from_square = __builtin_ctzll(from_bitboard);
    uint8_t to_square = __builtin_ctzll(to_bitboard);
    uint8_t check_square = __builtin_ctzll(check_square_bitboard);

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            SDL_Rect square = {x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};

            // Draw square background
            if ((x + y) % 2 == 0) { 
                SDL_SetRenderDrawColor(renderer, light.r, light.g, light.b, light.a); } 
            else { 
                SDL_SetRenderDrawColor(renderer, dark.r, dark.g, dark.b, dark.a); }

            if (x + 8 * y == from_square) {
                SDL_SetRenderDrawColor(renderer, from.r, from.g, from.b, from.a);
            } else if (x + 8 * y == to_square) {
                SDL_SetRenderDrawColor(renderer, to.r, to.g, to.b, to.a);
            }
            if ((x + 8 * y == check_square) && (check_square_bitboard != 1)) {
                SDL_SetRenderDrawColor(renderer, check.r, check.g, check.b, check.a);
            }

            SDL_RenderFillRect(renderer, &square);

            int texture_index = piece_char_to_index(board[y][x]);
            if (texture_index >= 0 && piece_textures[texture_index] != NULL) {
                // Create smaller rectangle for piece with padding
                SDL_Rect piece_rect = {
                    x * SQUARE_SIZE + PADDING,
                    y * SQUARE_SIZE + PADDING,
                    SQUARE_SIZE - (2 * PADDING),
                    SQUARE_SIZE - (2 * PADDING)
                };
                SDL_RenderCopy(renderer, piece_textures[texture_index], NULL, &piece_rect);
            }
        }
    }
}

// Thread function for SDL GUI loop
void* sdl_gui_loop(void* arg) 
{
    GUI_Args_t* gui_args = (GUI_Args_t*)arg;
    Position_t* current_position = gui_args->position;
    Position_t previous_position = *current_position;
    playing_as_white = (gui_args->playing_as_white);

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("TessMax!!",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    load_piece_textures(renderer);

    int running = 1;
    SDL_Event event;

    while (running) {
        // exit event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = 0; }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        position_to_board_array(current_position, board);
        render_chess_board(renderer);

        if (is_different(current_position, &previous_position)) {
            find_difference(current_position, &previous_position, &from_bitboard, &to_bitboard);
            print_bitboard(from_bitboard);
            print_bitboard(to_bitboard);
            previous_position = *current_position; // Update previous position
            if (is_check(current_position)) {
                check_square_bitboard = current_position->pieces[current_position->white_to_move].kings;
            } else {
                check_square_bitboard = 1; // No check
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    }

    // Clean up textures
    for (int i = 0; i < 12; i++) {
        if (piece_textures[i] != NULL) {
            SDL_DestroyTexture(piece_textures[i]);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return NULL;
}

