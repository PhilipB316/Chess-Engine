// File: gui.c

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "gui.h"
#include "log.h"
#include "../movefinding/board.h"
#include "../ui/movedisplay.h"
#include "../search/evaluate.h"

static char board[BOARD_SIZE][BOARD_SIZE] = {0};
static SDL_Texture* piece_textures[12] = {NULL};

static char move_notation[MOVE_NOTATION_LENGTH * MAXIMUM_GAME_LENGTH] = {0};

static bool* playing_as_white; // Default perspective for printing the board
static ULL from_bitboard, to_bitboard;
static ULL check_square_bitboard = 1;

static SDL_Color light = {240, 217, 181, 255};        // Standard light squares
static SDL_Color dark = {181, 136, 99, 255};          // Standard dark squares
static SDL_Color to = {210, 180, 140, 255};    // Tan for dark "from"
static SDL_Color from = {147, 112, 219, 255};      // Medium slate blue for dark "to"
static SDL_Color check = {255, 0, 0, 255}; // Red for check square


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

void cleanup_gui(SDL_Renderer* renderer, SDL_Window* window);

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
        case 'P': return 0; // white pawn
        case 'N': return 1; // white knight
        case 'B': return 2; // white bishop
        case 'R': return 3; // white rook
        case 'Q': return 4; // white queen
        case 'K': return 5; // white king
        case 'p': return 6; // black pawn
        case 'n': return 7; // black knight
        case 'b': return 8; // black bishop
        case 'r': return 9; // black rook
        case 'q': return 10; // black Queen
        case 'k': return 11; // black King
        default: return -1; // empty square or unknown piece
    }
}

void position_to_board_array(Position_t* position, 
                             char board[BOARD_SIZE][BOARD_SIZE]) 
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

        if (white_pieces.pawns & (1ULL << i)) 
            { board[display_rank][display_file] = 'P'; }
        else if (white_pieces.knights & (1ULL << i)) 
            { board[display_rank][display_file] = 'N'; }
        else if (white_pieces.bishops & (1ULL << i)) 
            { board[display_rank][display_file] = 'B'; }
        else if (white_pieces.rooks & (1ULL << i)) 
            { board[display_rank][display_file] = 'R'; }
        else if (white_pieces.queens & (1ULL << i)) 
            { board[display_rank][display_file] = 'Q'; }
        else if (white_pieces.kings & (1ULL << i)) 
            { board[display_rank][display_file] = 'K'; }
        else if (black_pieces.pawns & (1ULL << i)) 
            { board[display_rank][display_file] = 'p'; }
        else if (black_pieces.knights & (1ULL << i)) 
            { board[display_rank][display_file] = 'n'; }
        else if (black_pieces.bishops & (1ULL << i)) 
            { board[display_rank][display_file] = 'b'; }
        else if (black_pieces.rooks & (1ULL << i)) 
            { board[display_rank][display_file] = 'r'; }
        else if (black_pieces.queens & (1ULL << i)) 
            { board[display_rank][display_file] = 'q'; }
        else if (black_pieces.kings & (1ULL << i)) 
            { board[display_rank][display_file] = 'k'; }
    }
}

void render_chess_board(SDL_Renderer* renderer)
{

    uint8_t from_square = __builtin_ctzll(from_bitboard);
    uint8_t to_square = __builtin_ctzll(to_bitboard);
    uint8_t check_square = __builtin_ctzll(check_square_bitboard);

    if (! *playing_as_white) { 
        // Adjust for black perspective
        from_square = 63 - from_square;
        to_square = 63 - to_square;
        check_square = 63 - check_square;
    }

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            SDL_Rect square = {x * SQUARE_SIZE, y * SQUARE_SIZE, 
                SQUARE_SIZE, SQUARE_SIZE};

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
                SDL_RenderCopy(renderer, piece_textures[texture_index], 
                               NULL, &piece_rect);
            }
        }
    }
}

void render_moves(SDL_Renderer* renderer) 
{
    SDL_SetRenderDrawColor(renderer, 220, 100, 100, 255); // Pale red background
    SDL_Rect move_rect = {BOARD_WIDTH, 0, BOARD_WIDTH + RIGHT_BORDER_WIDTH, 
        BOARD_HEIGHT + BOTTOM_BORDER_HEIGHT};
    SDL_RenderFillRect(renderer, &move_rect);

    if (move_notation[0] == '\0') { return; }

    SDL_Color textColor = {0, 0, 0, 255}; // Black color
    TTF_Font* font = TTF_OpenFont("../assets/font/DejaVuSansMono.ttf", FONT_SIZE);
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended_Wrapped(
        font, move_notation, textColor, move_rect.w - 20);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {move_rect.x + 10, move_rect.y + 10, 
        textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

void update_notation_string(Position_t* current_position, Position_t* previous_position) 
{
    static uint8_t move_count = 0;
    static char last_white_move[MOVE_NOTATION_LENGTH] = {0};
    static char last_black_move[MOVE_NOTATION_LENGTH] = {0};
    char move[MOVE_NOTATION_LENGTH] = {0};
    get_move_notation(previous_position, current_position, move);

    if (move_count % 2 == 0) { // whites move
        strcat(move_notation, "\n");
        // Shift moves if exceeding max display limit
        if (move_count > MAX_NUM_MOVES_ON_GUI) {
            memmove(move_notation, move_notation + MOVE_NOTATION_LENGTH, 
                    (MAX_NUM_MOVES_ON_GUI - 1) * MOVE_NOTATION_LENGTH);
            move_notation[(MAX_NUM_MOVES_ON_GUI - 1) * MOVE_NOTATION_LENGTH] = '\0';
        }
        // Add move number
        char move_count_str[8];
        sprintf(move_count_str, "%2d.   ", (move_count / 2 + 1));
        strcat(move_notation, move_count_str);
        // Store last white move
        strncpy(last_white_move, move, MOVE_NOTATION_LENGTH);
        last_white_move[MOVE_NOTATION_LENGTH - 1] = '\0';
    } else { // blacks move
        strcat(move_notation, " ");
        strncpy(last_black_move, move, MOVE_NOTATION_LENGTH);
        last_black_move[MOVE_NOTATION_LENGTH - 1] = '\0';
        // write entire move line to log file
        char log_msg[2 * MOVE_NOTATION_LENGTH + 17];
        sprintf(log_msg, "%d. %s %s\n", (move_count / 2 + 1), 
                last_white_move, last_black_move);
        log_message(log_msg);
    }
    char padded_move[8];
    sprintf(padded_move, "%-6s", move);
    strcat(move_notation, padded_move);
    move_count++;
}

void update_square_highlighting(Position_t *current_position, 
                                Position_t previous_position)
{

    find_from_to_square(&previous_position, current_position, 
                        &from_bitboard, &to_bitboard);
    if (is_check(current_position, current_position->white_to_move)) {
        check_square_bitboard = current_position->pieces[
            current_position->white_to_move].kings;
    } else { check_square_bitboard = 1; /* No check */ }
}

void* sdl_gui_loop(void* arg)
{
    GUI_Args_t* gui_args = (GUI_Args_t*)arg;
    Position_t* current_position = gui_args->position;
    Position_t previous_position = *current_position;
    playing_as_white = (gui_args->playing_as_white);

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        exit(1);
    }

    move_notation[0] = '\0'; // Initialize move notation string

    SDL_Window* window = SDL_CreateWindow("TessMax!!",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          BOARD_WIDTH + RIGHT_BORDER_WIDTH,
                                          BOARD_HEIGHT,
                                          SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    load_piece_textures(renderer);

    int running = 1;
    SDL_Event event;

    while (running) {
        // exit event handling
        while (SDL_PollEvent(&event)) { if (event.type == SDL_QUIT) { running = 0; } }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer); // clear the renderer
        position_to_board_array(current_position, board);
        render_chess_board(renderer);
        render_moves(renderer);

        // check for changes in position and update highlighting accordingly
        if (is_different(current_position, &previous_position)) {
            update_notation_string(current_position, &previous_position);
            update_square_highlighting(current_position, previous_position);
            previous_position = *current_position; // Update previous position
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    }
    cleanup_gui(renderer, window);
    return NULL;
}

void cleanup_gui(SDL_Renderer* renderer, SDL_Window* window) 
{
    for (int i = 0; i < 12; i++) {
        if (piece_textures[i]) {
            SDL_DestroyTexture(piece_textures[i]);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

