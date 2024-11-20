#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "position.h"

static uint8_t possible_knight_move_table[64][8] = {
    {10, 17, -1, -1, -1, -1, -1, -1}, // 0
    {11, 18, 16, -1, -1, -1, -1, -1},
    {8, 17, 19, 12, -1, -1, -1, -1},
    {9, 18, 20, 13, -1, -1, -1, -1},
    {10, 19, 21, 14, -1, -1, -1, -1},
    {11, 20, 22, 15, -1, -1, -1, -1},
    {12, 21, 23, -1, -1, -1, -1, -1},
    {13, 22, -1, -1, -1, -1, -1, -1},

    {2, 18, 25, -1, -1, -1, -1, -1},   // 8
    {3, 19, 24, 26, -1, -1, -1, -1},
    {0, 4, 20, 25, 27, 16, -1, -1},
    {1, 5, 21, 26, 28, 17, -1, -1},
    {2, 6, 22, 27, 29, 18, -1, -1},
    {3, 7, 23, 28, 30, 19, -1, -1},
    {4, 8, 29, 31, 20, -1, -1, -1},
    {5, 30, 21, -1, -1, -1, -1, -1},

    {1, 10, 26, 33, -1, -1, -1, -1},  // 16
    {0, 2, 11, 27, 32, 34, -1, -1},
    {1, 3, 12, 28, 33, 35, 8, 24},
    {2, 4, 13, 29, 34, 36, 9, 25},
    {3, 5, 14, 30, 35, 37, 10, 26},
    {4, 6, 15, 31, 36, 38, 11, 27},
    {5, 7, 37, 39, 12, 28, -1, -1},
    {6, 38, 13, 29, -1, -1, -1, -1},

    {9, 18, 34, 41, -1, -1, -1, -1},  //24
    {8, 10, 18, 35, 40, 42, -1, -1},
    {9, 11, 20, 36, 41, 43, 16, 32},
    {10, 12, 21, 37, 42, 44, 17, 33},
    {11, 13, 22, 38, 43, 45, 18, 34},
    {12, 14, 23, 39, 44, 46, 19, 35},
    {13, 15, 45, 47, 20, 36, -1, -1},
    {14, 46, 21, 37, -1, -1, -1, -1},

    {17, 26, 42, 49, -1, -1, -1, -1},  // 32
    {16, 18, 27, 43, 48, 50, -1, -1},
    {17, 19, 28, 44, 49, 51, 24, 40},
    {18, 20, 29, 45, 50, 52, 25, 41},
    {19, 21, 30, 46, 51, 53, 26, 42},
    {20, 22, 31, 47, 52, 54, 27, 43},
    {21, 23, 53, 55, 28, 44, -1, -1},
    {22, 54, 29, 45, -1, -1, -1, -1},

    {25, 34, 50, 57, -1, -1, -1, -1},  // 40
    {24, 26, 35, 51, 56, 58, -1, -1},
    {25, 27, 36, 52, 57, 59, 32, 48},
    {26, 28, 37, 53, 58, 60, 33, 49},
    {27, 29, 38, 54, 59, 61, 34, 50},
    {28, 30, 39, 55, 60, 62, 35, 51},
    {29, 31, 61, 63, 36, 52, -1, -1},
    {30, 62, 37, 53, -1, -1, -1, -1},

    {33, 42, 58, -1, -1, -1, -1, -1},  // 48
    {32, 34, 43, 59, -1, -1, -1, -1},
    {33, 35, 44, 60, 40, 56, -1, -1},
    {34, 36, 45, 61, 41, 57, -1, -1},
    {35, 37, 46, 62, 42, 58, -1, -1},
    {36, 38, 47, 63, 43, 59, -1, -1},
    {37, 39, 60, 44, -1, -1, -1, -1},
    {38, 45, 61, -1, -1, -1, -1, -1},

    {41, 50, -1, -1, -1, -1, -1, -1},  // 56
    {40, 42, 51, -1, -1, -1, -1, -1},
    {41, 43, 52, 48, -1, -1, -1, -1},
    {42, 44, 53, 49, -1, -1, -1, -1},
    {43, 45, 54, 50, -1, -1, -1, -1},
    {44, 46, 55, 51, -1, -1, -1, -1},
    {45, 47, 52, -1, -1, -1, -1, -1},
    {46, 53, -1, -1, -1, -1, -1, -1}
    };


Position_t fen_to_board(char fen[])
{   
    char board[64] = {0};
    memset(board, ' ', sizeof(board));
    Move_t moves[MOVE_LIST_SIZE] = {0};

    size_t i = 1;
    uint16_t square_counter = 0;

    char character = fen[0];

    while (character != ' ')
    {
        if (isalpha(character)) {
            board[square_counter++] = character;
        }
        else if (isdigit(character)) {
            square_counter += character - '0';
        } 
        character = fen[i++];
    }

    Position_t position;
    memcpy(position.board, board, sizeof(board));
    memcpy(position.moves, moves, sizeof(moves));

    return position;
}


bool not_on_edge(uint8_t square)
{
    return square % 8 != 0 && square % 8 != 7 && square / 8 != 0 && square / 8 != 7;
}


void print_board(char board[])
{
    for (size_t i = 0; i < 64; i++)
    {
        printf("%c", board[i]);
        if ((i + 1) % 8 == 0)
        {
            printf("\n");
        }
    }
}


void print_moves(Position_t* position)
{
    Move_t* moves = position->moves;
    size_t num_possible_moves = position->num_possible_moves;

    for (size_t i = 0; i < num_possible_moves; i++)
    {
        printf("%c: %d -> %d\n", moves[i].piece, moves[i].location, moves[i].destination);
    }
}


void determine_possible_directional_moves(bool is_piece_white, size_t square, char* board, Move_t* moves, size_t* move_counter, char piece, int* possible_directions, size_t num_directions)
{
    for (size_t i = 0; i < num_directions; i++)
    {
        int direction = possible_directions[i];
        uint8_t current_square = square;
        while (true)
        {
            current_square += direction;
            if (board[current_square] == ' ')
            {
                moves[*move_counter].piece = piece;
                moves[*move_counter].location = square;
                moves[*move_counter].destination = current_square;
                (*move_counter)++;
                if (!not_on_edge(current_square))
                {
                    break;
                }
            } else if (is_piece_white && islower(board[current_square]))
            {
                moves[*move_counter].piece = piece;
                moves[*move_counter].location = square;
                moves[*move_counter].destination = current_square;
                (*move_counter)++;
                break;
            } else if (!is_piece_white && isupper(board[current_square]))
            {
                moves[*move_counter].piece = piece;
                moves[*move_counter].location = square;
                moves[*move_counter].destination = current_square;
                (*move_counter)++;
                break;
            } else
            {
                break;
            }
        }
    }
}


void determine_possible_directional_threats(bool is_piece_white, size_t square, char* board, uint8_t* threats, size_t* threat_counter, int* possible_directions, size_t num_directions)
{
    for (size_t i = 0; i < num_directions; i++)
    {
        int direction = possible_directions[i];
        uint8_t current_square = square;
        while (true)
        {
            current_square += direction;
            if (board[current_square] == ' ')
            {
                threats[*threat_counter] = current_square;
                (*threat_counter)++;
                if (!not_on_edge(current_square))
                {
                    break;
                }
            } else if (is_piece_white && islower(board[current_square]))
            {
                threats[*threat_counter] = current_square;
                (*threat_counter)++;
                break;
            } else if (!is_piece_white && isupper(board[current_square]))
            {
                threats[*threat_counter] = current_square;
                (*threat_counter)++;
                break;
            } else
            {
                break;
            }
        }
    }
}


void diagonal_directions(uint8_t row, uint8_t col, int* possible_directions, size_t* num_directions)
{
    if (row == 0 && col == 0) {
        possible_directions[0] = 9;
        *num_directions = 1;
    } else if (row == 0 && col == 7) {
        possible_directions[0] = 7;
        *num_directions = 1;
    } else if (row == 7 && col == 0) {
        possible_directions[0] = -7;
        *num_directions = 1;
    } else if (row == 7 && col == 7) {
        possible_directions[0] = -9;
        *num_directions = 1;
    } else if (row == 0) {
        possible_directions[0] = 7;
        possible_directions[1] = 9;
        *num_directions = 2;
    } else if (row == 7) {
        possible_directions[0] = -7;
        possible_directions[1] = -9;
        *num_directions = 2;
    } else if (col == 0) {
        possible_directions[0] = -7;
        possible_directions[1] = 9;
        *num_directions = 2;
    } else if (col == 7) {
        possible_directions[0] = -9;
        possible_directions[1] = 7;
        *num_directions = 2;
    } else {
        possible_directions[0] = -9;
        possible_directions[1] = -7;
        possible_directions[2] = 7;
        possible_directions[3] = 9;
        *num_directions = 4;
    }
}


void straight_directions(uint8_t row, uint8_t col, int* possible_directions, size_t* num_directions)
{
    if (row == 0 && col == 0) {
        possible_directions[0] = 1;
        possible_directions[1] = 8;
        *num_directions = 2;
    } else if (row == 0 && col == 7) {
        possible_directions[0] = -1;
        possible_directions[1] = 8;
        *num_directions = 2;
    } else if (row == 7 && col == 0) {
        possible_directions[0] = 1;
        possible_directions[1] = -8;
        *num_directions = 2;
    } else if (row == 7 && col == 7) {
        possible_directions[0] = -1;
        possible_directions[1] = -8;
        *num_directions = 2;
    } else if (row == 0) {
        possible_directions[0] = -1;
        possible_directions[1] = 1;
        possible_directions[2] = 8;
        *num_directions = 3;
    } else if (row == 7) {
        possible_directions[0] = -1;
        possible_directions[1] = 1;
        possible_directions[2] = -8;
        *num_directions = 3;
    } else if (col == 0) {
        possible_directions[0] = 1;
        possible_directions[1] = -8;
        possible_directions[2] = 8;
        *num_directions = 3;
    } else if (col == 7) {
        possible_directions[0] = -1;
        possible_directions[1] = -8;
        possible_directions[2] = 8;
        *num_directions = 3;
    } else {
        possible_directions[0] = -1;
        possible_directions[1] = 1;
        possible_directions[2] = -8;
        possible_directions[3] = 8;
        *num_directions = 4;
    }
}


void pawn_moves(bool is_piece_white, size_t square, char* board, Move_t* moves,  size_t* move_counter)
{
    int direction = is_piece_white ? -1 : 1;
    int start_row = is_piece_white ? 6 : 1;
    int row = square / 8;
    int col = square % 8;
    char piece = is_piece_white ? 'P' : 'p';

    if (board[square + direction * 8] == ' ')
    {
        // Single move
        moves[*move_counter].piece = piece;
        moves[*move_counter].location = square;
        moves[*move_counter].destination = square + direction * 8;
        (*move_counter)++;

        // Double move from starting position
        if (row == start_row && board[square + direction * 16] == ' ')
        {
            moves[*move_counter].piece = piece;
            moves[*move_counter].location = square;
            moves[*move_counter].destination = square + direction * 16;
            (*move_counter)++;
        }
    }
    // Capture moves
    if (is_piece_white)
    {
        if (col > 0 && islower(board[square + direction * 9]))
        {
            moves[*move_counter].piece = piece;
            moves[*move_counter].location = square;
            moves[*move_counter].destination = square + direction * 9;
            (*move_counter)++;
        }
        if (col < 7 && islower(board[square + direction * 7]))
        {
            moves[*move_counter].piece = piece;
            moves[*move_counter].location = square;
            moves[*move_counter].destination = square + direction * 7;
            (*move_counter)++;
        }
    } else
    {
        if (col > 0 && isupper(board[square + direction * 7]))
        {
            moves[*move_counter].piece = piece;
            moves[*move_counter].location = square;
            moves[*move_counter].destination = square + direction * 7;
            (*move_counter)++;
        }
        if (col < 7 && isupper(board[square + direction * 9]))
        {
            moves[*move_counter].piece = piece;
            moves[*move_counter].location = square;
            moves[*move_counter].destination = square + direction * 9;
            (*move_counter)++;
        }
    }

}


void pawn_threats(bool is_piece_white, size_t square, uint8_t* threats, size_t* threat_counter)
{
    int direction = is_piece_white ? -1 : 1;
    int col = square % 8;
    if (is_piece_white)
    {
        if (col > 0)
        {
            threats[*threat_counter] = square + direction * 9;
            (*threat_counter)++;
        }
        if (col < 7)
        {
            threats[*threat_counter] = square + direction * 7;
            (*threat_counter)++;
        }
    } else
    {
        if (col > 0)
        {
            threats[*threat_counter] = square + direction * 7;
            (*threat_counter)++;
        }
        if (col < 7)
        {
            threats[*threat_counter] = square + direction * 9;
            (*threat_counter)++;
        }
    }
}    


void knight_threats(bool is_piece_white,size_t square, uint8_t* threats, size_t* threat_counter)
{
    uint8_t* possible_moves = possible_knight_move_table[square];
    for (size_t i = 0; i < 8; i++)
    {
        if (possible_moves[i] == -1)
        {
            break;
        }
        threats[*threat_counter] = possible_moves[i];
        (*threat_counter)++;
    }
}


void knight_moves(bool is_piece_white, size_t square, char* board, Move_t* moves, size_t* move_counter)
{
    char piece = is_piece_white ? 'N' : 'n';
    uint8_t* possible_moves = possible_knight_move_table[square];
    for (size_t i = 0; i < 8; i++)
    {
        if (possible_moves[i] == -1)
        {
            break;
        }
        if (board[possible_moves[i]] == ' ')
        {
            moves[*move_counter].piece = piece;
            moves[*move_counter].location = square;
            moves[*move_counter].destination = possible_moves[i];
            (*move_counter)++;
        } else if (is_piece_white && islower(board[possible_moves[i]]))
        {
            moves[*move_counter].piece = 'N';
            moves[*move_counter].location = square;
            moves[*move_counter].destination = possible_moves[i];
            (*move_counter)++;
        } else if (!is_piece_white && isupper(board[possible_moves[i]]))
        {
            moves[*move_counter].piece = 'n';
            moves[*move_counter].location = square;
            moves[*move_counter].destination = possible_moves[i];
            (*move_counter)++;
        }        
    }   
}


void diagonal_threats(bool is_piece_white, size_t square, char* board, uint8_t* threats, size_t* threat_counter)
{
    int row = square / 8;
    int col = square % 8;

    int possible_directions[4];
    size_t num_directions;

    diagonal_directions(row, col, possible_directions, &num_directions);
    determine_possible_directional_threats(is_piece_white, square, board, threats, threat_counter, possible_directions, num_directions);
}


void diagonal_moves(bool is_piece_white, size_t square, char* board, Move_t* move, size_t* move_counter, char piece)
{
    int row = square / 8;
    int col = square % 8;

    int possible_directions[4];
    size_t num_directions;

    diagonal_directions(row, col, possible_directions, &num_directions);
    determine_possible_directional_moves(is_piece_white, square, board, move, move_counter, piece, possible_directions, num_directions);
        
}


void straight_threats(bool is_piece_white, size_t square, char* board, uint8_t* threats, size_t* threat_counter)
{
    int row = square / 8;
    int col = square % 8;

    int possible_directions[4];
    size_t num_directions;

    straight_directions(row, col, possible_directions, &num_directions);
    determine_possible_directional_threats(is_piece_white, square, board, threats, threat_counter, possible_directions, num_directions);
}


void straight_moves(bool is_piece_white, size_t square, char* board, Move_t* move, size_t* move_counter, char piece)
{
    int row = square / 8;
    int col = square % 8;
    int possible_directions[4];
    size_t num_directions;

    straight_directions(row, col, possible_directions, &num_directions);
    determine_possible_directional_moves(is_piece_white, square, board, move, move_counter, piece, possible_directions, num_directions);
}


void possible_king_moves(uint8_t square, uint8_t* possible_squares, size_t* num_squares)
{
    int row = square / 8;
    int col = square % 8;
    size_t counter = 0;
    if (row > 0)
    {
        possible_squares[counter++] = square - 8;
        if (col > 0)
        {
            possible_squares[counter++] = square - 9;
        }
        if (col < 7)
        {
            possible_squares[counter++] = square - 7;
        }
    }
    if (row < 7)
    {
        possible_squares[counter++] = square + 8;
        if (col > 0)
        {
            possible_squares[counter++] = square + 7;
        }
        if (col < 7)
        {
            possible_squares[counter++] = square + 9;
        }
    }
    if (col > 0)
    {
        possible_squares[counter++] = square - 1;
    }
    if (col < 7)
    {
        possible_squares[counter++] = square + 1;
    }
    *num_squares = counter;
}

void print_threatened_squares(uint8_t* threats, size_t num_threats)
{
    for (size_t i = 0; i < num_threats; i++)
    {
        printf("%d ", threats[i]);
    }
    printf("\n");
}


void determine_possible_moves(Position_t* position)
{
    char* board = position->board;
    Move_t* moves = position->moves;
    size_t* num_possible_moves = &position->num_possible_moves;
    size_t move_counter = 0;
    bool is_white_turn = position->is_white_turn;

    uint8_t threats[300] = {0};
    size_t threat_counter = 0;
    uint8_t king_square;

    for (size_t square = 0; square < 64; square++)
    {
        char piece = board[square];
        bool is_piece_white = isupper(piece);
        char lowered_piece = tolower(piece);
        
        if (is_piece_white == is_white_turn)
        {
            if (lowered_piece == 'p')
            {
                pawn_moves(is_piece_white, square, board, moves, &move_counter);
            } else if (lowered_piece == 'n')
            {
                knight_moves(is_piece_white, square, board, moves, &move_counter);
            } else if (lowered_piece == 'b')
            {
                diagonal_moves(is_piece_white, square, board, moves, &move_counter, piece);
            } else if (lowered_piece == 'r')
            {
                straight_moves(is_piece_white, square, board, moves, &move_counter, piece);
            } else if (lowered_piece == 'q')
            {
                diagonal_moves(is_piece_white, square, board, moves, &move_counter, piece);
                straight_moves(is_piece_white, square, board, moves, &move_counter, piece);
            } else if (lowered_piece == 'k')
            {
                king_square = square;
            }
        } else {
            if (lowered_piece == 'p')
            {
                pawn_threats(is_piece_white, square, threats, &threat_counter);
            } else if (lowered_piece == 'n')
            {
                knight_threats(is_piece_white, square, threats, &threat_counter);
            } else if (lowered_piece == 'b')
            {
                diagonal_threats(is_piece_white, square, board, threats, &threat_counter);
            } else if (lowered_piece == 'r')
            {
                straight_threats(is_piece_white, square, board, threats, &threat_counter);
            } else if (lowered_piece == 'q')
            {
                diagonal_threats(is_piece_white, square, board, threats, &threat_counter);
                straight_threats(is_piece_white, square, board, threats, &threat_counter);
            } else if (lowered_piece == 'k')
            {
                uint8_t possible_squares[8] = {0};
                size_t num_possible_squares = 0;
                possible_king_moves(square, possible_squares, &num_possible_squares);
                for (size_t i = 0; i < num_possible_squares; i++)
                {
                    threats[threat_counter] = possible_squares[i];
                }
            }
            uint8_t possible_squares[8] = {0};
            size_t num_possible_squares = 0;
            possible_king_moves(square, possible_squares, &num_possible_squares);
            for (size_t i = 0; i < num_possible_squares; i++)
            {
                uint8_t king_move_square = possible_squares[i];
                for (size_t j = 0; j < threat_counter; j++)
                {
                    if (threats[j] == king_move_square)
                    {
                        break;
                    }
                    else
                    {
                        if (is_piece_white)
                        {
                            if (islower(board[king_move_square]) || board[king_move_square] == ' ')
                            {
                                moves[move_counter].piece = 'K';
                                moves[move_counter].location = king_square;
                                moves[move_counter].destination = king_move_square;
                                move_counter++;
                            }
                        } else
                        {
                            if (isupper(board[king_move_square]) || board[king_move_square] == ' ')
                            {
                                moves[move_counter].piece = 'k';
                                moves[move_counter].location = king_square;
                                moves[move_counter].destination = king_move_square;
                                move_counter++;
                            }
                        }
                    }
                }
            }
        }
    }
    *num_possible_moves = move_counter;
    //  print_threatened_squares(threats, threat_counter);
}

