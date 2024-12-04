#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"
#include "move_lookuptables.h"


void print_bitboard(uint64_t bitboard)
{
    for (uint8_t i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
        {
            // pretty printing for the ranks:
            printf("\n");
            printf("%u   ", 8 - i / 8);
        }
        if (bitboard & (1ULL << i))
        {
            printf("1 ");
        }
        else
        {
            printf(". ");
        }
    }
    // pretty printing for the files:
    printf("\n\n    a b c d e f g h\n\n");
}


Position_t fen_to_board(char* fen)
{   
    Position_t fen_position = {0};
    size_t i = 0;  // i does not necessarily count up to 64
    uint8_t square_counter = 0;  // square_counter counts up to 64

    char character = fen[i++];

    while (character != ' ')  // stops and end of board representation not complete FEN
    {
        if (isalpha(character)) {
            if (isupper(character)) {
                if (character == 'K') {
                    fen_position.white_pieces.kings |= (1ULL << square_counter);
                } else if (character == 'Q') {
                    fen_position.white_pieces.queens |= (1ULL << square_counter);
                } else if (character == 'R') {
                    fen_position.white_pieces.rooks |= (1ULL << square_counter);
                } else if (character == 'B') {
                    fen_position.white_pieces.bishops |= (1ULL << square_counter);
                } else if (character == 'N') {
                    fen_position.white_pieces.knights |= (1ULL << square_counter);
                } else if (character == 'P') {
                    fen_position.white_pieces.pawns |= (1ULL << square_counter);
                }
                fen_position.white_pieces.all_pieces |= (1ULL << square_counter);
            } else {
                if (character == 'k') {
                    fen_position.black_pieces.kings |= (1ULL << square_counter);
                } else if (character == 'q') {
                    fen_position.black_pieces.queens |= (1ULL << square_counter);
                } else if (character == 'r') {
                    fen_position.black_pieces.rooks |= (1ULL << square_counter);
                } else if (character == 'b') {
                    fen_position.black_pieces.bishops |= (1ULL << square_counter);
                } else if (character == 'n') {
                    fen_position.black_pieces.knights |= (1ULL << square_counter);
                } else if (character == 'p') {
                    fen_position.black_pieces.pawns |= (1ULL << square_counter);
                }
                fen_position.black_pieces.all_pieces |= (1ULL << square_counter);
            }
            fen_position.all_pieces |= (1ULL << square_counter);
            square_counter++;
        }
        else if (isdigit(character)) {
            square_counter += character - '0';
        } 
        character = fen[i++];
    }
    return fen_position;
}


void generate_moves_from_bitboard(Move_t* move_list, 
                                  size_t* num_moves, 
                                  bool white_to_move,
                                  PiecesOneColour_t* const opponent_pieces,
                                  Piece_t piece,
                                  uint8_t from_square,
                                  ULL possible_moves)
{
    while (possible_moves) {
        uint8_t to_square = __builtin_ctzll(possible_moves);
        (*num_moves)++;
        move_list[*num_moves].is_white = white_to_move;
        move_list[*num_moves].moved = piece;
        move_list[*num_moves].from_square = from_square;
        move_list[*num_moves].to_square = to_square;
        ULL square = 1ULL << to_square;
        if (opponent_pieces->all_pieces & square) {
            if (opponent_pieces->pawns & square) {
                move_list[*num_moves].captured = PAWN;
            } else if (opponent_pieces->knights & square) {
                move_list[*num_moves].captured = KNIGHT;
            } else if (opponent_pieces->bishops & square) {
                move_list[*num_moves].captured = BISHOP;
            } else if (opponent_pieces->rooks & square) {
                move_list[*num_moves].captured = ROOK;
            } else if (opponent_pieces->queens & square) {
                move_list[*num_moves].captured = QUEEN;
            } else if (opponent_pieces->kings & square) {
                move_list[*num_moves].captured = KING;
            }
        } else {
            move_list[*num_moves].captured = NONE;
        }
        possible_moves &= ~square;

    }
}

void queen_move_finder(Move_t* move_list, 
                       size_t* num_moves, 
                       Position_t* const position)
{
    PiecesOneColour_t* active_pieces = position->white_to_move ? &position->black_pieces : &position->white_pieces;
    ULL queen_bitboard = active_pieces->queens;

    while (queen_bitboard != 0) {
        // printf("done\n");

        uint8_t from_square = __builtin_ctzll(queen_bitboard);
        ULL possible_moves = 0;
        ULL all_pieces = position->all_pieces;
        ULL rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        ULL bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        uint16_t index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_moves |= rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_moves |= bishop_attack_lookup_table[from_square][index];
        possible_moves &= ~active_pieces->all_pieces;        

        generate_moves_from_bitboard(move_list, 
                                     num_moves, 
                                     position->white_to_move, 
                                     active_pieces, 
                                     QUEEN, 
                                     from_square, 
                                     possible_moves);
                                     
        queen_bitboard &= ~(1ULL << (from_square));
    }
}


void print_moves(Move_t* move_list, size_t* num_moves)
{
    for (size_t i = 0; i < *num_moves; i++)
    {
        switch (move_list[i].moved)
        {
        case 1:
            /* code */
            break;
        case 2:
            printf("N");
            break;
        case 3:
            printf("B");
            break;
        case 4:
            printf("R");
            break;
        case 5:
            printf("Q");
            break;
        case 6:
            printf("K");
            break;
        }
        printf("%s\n", pretty_print_moves[move_list[i].from_square]);
    }
        
}
