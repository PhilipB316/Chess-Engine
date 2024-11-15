#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

// #include "move_finder.h"
#include "position.h"

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


void print_moves(Move_t* moves, size_t num_possible_moves)
{
    for (size_t i = 0; i < num_possible_moves; i++)
    {
        printf("%c: %d -> %d\n", moves[i].piece, moves[i].location, moves[i].destination);
    }
}

void pawn_moves(bool is_white, size_t square, char* board, Move_t* moves, size_t* move_counter)
{
    int direction = is_white ? -1 : 1;
    int start_row = is_white ? 6 : 1;
    int row = square / 8;
    int col = square % 8;
    char piece = is_white ? 'P' : 'p';

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
    if (is_white)
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


void determine_possible_moves(Position_t* position)
{
    char* board = position->board;
    Move_t* moves = position->moves;
    size_t* num_possible_moves = &position->num_possible_moves;
    size_t move_counter = 0;

    for (size_t square = 0; square < 64; square++)
    {
        char piece = board[square];
        bool is_white = isupper(piece);
        piece = tolower(piece);
        
        if (piece == 'p')
        {
            pawn_moves(is_white, square, board, moves, &move_counter);
        }
    }
    *num_possible_moves = move_counter;
}
