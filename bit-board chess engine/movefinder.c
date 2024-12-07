/**
 * @file movefinder.c
 * @brief This file implements move generation
 * @author Philip Brand
 * @date 2024-12-06
 * 
 */




#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"
#include "move_lookuptables.h"

static Move_t* MOVE_LIST;
static size_t* NUM_MOVES;
static bool WHITE_TO_MOVE;
static PiecesOneColour_t* OPPONENT_PIECES;


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


void print_moves(Move_t* move_list, size_t num_moves)
{
    for (size_t i = 0; i < num_moves; i++)
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
        if (move_list[i].captured)
        {
            printf("x");
        }
        printf("%s\n", pretty_print_moves[move_list[i].to_square]);
    }
        
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



/**
 * @brief Generates the possible moves objects for each bit in a bitboard
 * 
 * @param MOVE_LIST The list of moves to be populated
 * @param NUM_MOVES The number of moves in the list
 * @param white_to_move boolean indicating if white is to move
 * @param OPPONENT_PIECES The bitboards of the opponent's pieces
 * @param piece The piece that is to be moved
 * @param from_square The square index of the piece
 * @param possible_moves The bitboard of possible moves
 * 
 */
void generate_moves_from_bitboard(Piece_t piece,
                                  uint8_t from_square,
                                  ULL possible_moves)
{
    while (possible_moves) {
        uint8_t to_square = __builtin_ctzll(possible_moves);
        MOVE_LIST[*NUM_MOVES].is_white = WHITE_TO_MOVE;
        MOVE_LIST[*NUM_MOVES].moved = piece;
        MOVE_LIST[*NUM_MOVES].from_square = from_square;
        MOVE_LIST[*NUM_MOVES].to_square = to_square;
        ULL square = 1ULL << to_square;
        // capture handling
        if (OPPONENT_PIECES->all_pieces & square) {  // global check for captures
            // then specifics for each piece
            if (OPPONENT_PIECES->pawns & square) {
                MOVE_LIST[*NUM_MOVES].captured = PAWN;
            } else if (OPPONENT_PIECES->knights & square) {
                MOVE_LIST[*NUM_MOVES].captured = KNIGHT;
            } else if (OPPONENT_PIECES->bishops & square) {
                MOVE_LIST[*NUM_MOVES].captured = BISHOP;
            } else if (OPPONENT_PIECES->rooks & square) {
                MOVE_LIST[*NUM_MOVES].captured = ROOK;
            } else if (OPPONENT_PIECES->queens & square) {
                MOVE_LIST[*NUM_MOVES].captured = QUEEN;
            } else if (OPPONENT_PIECES->kings & square) {
                MOVE_LIST[*NUM_MOVES].captured = KING;
            }
        } else {
            MOVE_LIST[*NUM_MOVES].captured = NONE;
        }
        (*NUM_MOVES)++;
        possible_moves &= ~square;
    }
}


void move_finder(Move_t* move_list, 
                size_t* num_moves, 
                Position_t* const position,
                PiecesOneColour_t* active_pieces,
                PiecesOneColour_t* opponent_pieces)
{
    ULL all_pieces = position->all_pieces;
    ULL rook_blockers, bishop_blockers;
    uint8_t from_square;
    ULL possible_moves = 0;
    uint16_t index;
    ULL not_active_pieces = ~active_pieces->all_pieces;
    WHITE_TO_MOVE = position->white_to_move;
    MOVE_LIST = move_list;
    NUM_MOVES = num_moves;
    OPPONENT_PIECES = opponent_pieces;

    // ========================= QUEENS =========================
    ULL queen_bitboard = active_pieces->queens;
    while (queen_bitboard) {
        from_square = __builtin_ctzll(queen_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_moves |= rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_moves |= bishop_attack_lookup_table[from_square][index];
        possible_moves &= not_active_pieces;    
        generate_moves_from_bitboard(QUEEN, from_square, possible_moves);
        queen_bitboard &= ~(1ULL << (from_square));
    }

    // ========================= ROOKS =========================
    ULL rook_bitboard = active_pieces->rooks;
    while (rook_bitboard) {
        from_square = __builtin_ctzll(rook_bitboard);
        possible_moves = 0;
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_moves |= rook_attack_lookup_table[from_square][index];
        possible_moves &= not_active_pieces;    
        generate_moves_from_bitboard(ROOK, from_square, possible_moves);
        rook_bitboard &= ~(1ULL << (from_square));
    }

    // ========================= BISHOPS =========================
    ULL bishop_bitboard = active_pieces->bishops;
    while (bishop_bitboard){
        from_square = __builtin_ctzll(bishop_bitboard);
        possible_moves = 0;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_moves |= bishop_attack_lookup_table[from_square][index];
        possible_moves &= not_active_pieces;
        generate_moves_from_bitboard(BISHOP, from_square, possible_moves);
        bishop_bitboard &= ~(1ULL << (from_square));
    }
}