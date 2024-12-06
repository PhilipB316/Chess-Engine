/**
 * @file movefinder.h
 * @brief movefinding, bitboard printing and move printing functions
 * @author Philip Brand
 * @date 2024-12-06
 * 
 * NOTE: bitboard and fen conversion functions not to be implemented here later.
 */

#ifndef MOVEFINDER_H
#define MOVEFINDER_H

#include <stdint.h>

#define ULL unsigned long long

typedef struct {
    ULL pawns;
    ULL knights;
    ULL bishops;
    ULL rooks;
    ULL queens;
    ULL kings;
    ULL all_pieces;
    bool castle_kingside;
    bool castle_queenside;
} PiecesOneColour_t;

typedef struct {
    PiecesOneColour_t white_pieces;
    PiecesOneColour_t black_pieces;
    ULL all_pieces;
    bool white_to_move;
} Position_t;

typedef enum {
    NONE = 0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} Piece_t;

/**
 * to move piece do XOR with bitboard corresponding to moved piece
 */
typedef struct {
    bool is_white;
    Piece_t captured;
    Piece_t moved;
    uint8_t from_square;
    uint8_t to_square;
} Move_t;


static char* pretty_print_moves[64] = 
{
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

/**
 * @brief Converts a FEN (Forsyth-Edwards Notation) string to a position,
 * represented as bitboards.
 *
 * @param fen The FEN string representing the position.
 * @return The board position represented as a Position_t structure.
 */
 Position_t fen_to_board(char fen[]);


/**
 * @brief Prints the bitboard.
 *
 * @param bitboard The bitboard to be printed.
 */
void print_bitboard(uint64_t bitboard);



/**
 * @brief Prints the moves in the move list.
 * 
 * Includes captures, currently does not include checks or checkmates.
 *
 * @param move_list The list of moves to be printed.
 * @param num_moves The number of moves in the list.
 */
void print_moves(Move_t* move_list, size_t* num_moves);


void queen_move_finder(Move_t* move_list, 
                       size_t* num_moves, 
                       Position_t* const position);




#endif