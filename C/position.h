#ifndef POSITION_H
#define POSITION_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "position.h"
// #include "move_finder.h"


#define MOVE_LIST_SIZE 1000


typedef struct
{
    char piece;
    uint8_t location;
    uint8_t destination;
} Move_t;

typedef struct
{
    char board[64];
    Move_t moves[MOVE_LIST_SIZE];
    size_t num_possible_moves;
    bool is_white_turn;
} Position_t;

/*
    * Converts a FEN string to board representation.
    * 
    * @param fen The FEN string to convert.
    * @return The board.
*/
Position_t fen_to_board(char* fen);

/*
    * Prints the board position in readable format.
    * 
    * @param board The board to print.
*/
void print_board(char* board);


void determine_possible_moves(Position_t* position);
void print_moves(Move_t* moves, size_t num_possible_moves);

#endif