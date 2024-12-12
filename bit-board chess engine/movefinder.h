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
#define NORMAL_MOVE 0
#define PROMOTION 1
#define CASTLING_KINGSIDE 2
#define CASTLING_QUEENSIDE 3

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
    uint16_t en_passant;
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
 * 
 * original_position is only set for first move in move list
 * 
 * special_flags are complicated:
 * 0: normal move
 * 1: pawn promotion to piece stored in moved
 * 2: kingside castling
 * 3: queenside castling
 * < 8 && < 100: the move is a pawn double push; the number is the possible square for capture
 * > 100: the move is an en passant capture; the number is the square of the pawn to be captured
 */
typedef struct {
    bool is_white;
    Piece_t captured;
    Piece_t moved;
    uint8_t special_flags;
    uint8_t from_square;
    uint8_t to_square;
    Position_t* original_position;
} Move_t;


// array of geometric letter/number pairs for each square
extern char* pretty_print_moves[64];


/**
 * @brief Converts a FEN (Forsyth-Edwards Notation) string to a position,
 * represented as bitboards.
 * 
 * Includes castling rights, en passant square, and side to move.
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
 * Includes captures, and promotions? currently does not include checks or checkmates.
 *
 * @param move_list The list of moves to be printed.
 * @param num_moves The number of moves in the list.
 */
void print_moves(Move_t* move_list, size_t num_moves);

/**
 * @brief Generates the possible moves objects for each piece in a bitboard
 * 
 * Finds move for all pieces: pawns, knights, bishops, rooks, queens, kings:
 * 
 * Queens, rooks, and bishops are found using magic numbers and lookup tables.
 * Knights use lookup tables.
 * 
 * Pawns are complicated, and use the following steps:
 * 1. determine possible attack squares
 * 2. determine normal captures
 * 3. determine if single push is possible
 * 4. if so, determine if double push is possible
 * 5. if double push is possible, immediately generate move,
 *      with possible en passant square as special flag
 * 6. check if pawn is on sevent rank;
 *    if so, generate moves with promotion as special flag
 * 7. otherwise, generate normal moves
 * 8. check if en passant capture is possible;
 *      if so, generate move with square of pawn being captured as special flag
 * 
 * Kings are also complicated, and use the following steps:
 * 1. determine squares covered by opponent pieces using magic numbers and lookup tables
 * 2. determine possible moves of own king using lookup tables,
 *      and remove threatened squares
 * 3. determine if castling is possible, using blockers masks non-threatened squares
 *  
 * 
 * @param move_list The list of moves to be populated
 * @param num_moves The number of moves in the list
 * @param position The current position
 * @param active_pieces The bitboards of the active player's pieces
 * @param opponent_pieces The bitboards of the opponent's pieces
 */
void move_finder(Move_t* move_list, 
                       size_t* num_moves, 
                       Position_t* const position,
                       PiecesOneColour_t* active_pieces,
                       PiecesOneColour_t* opponent_pieces);



void move_maker(Position_t* old_position, Position_t* new_position, Move_t* move);

void print_position(Position_t const position);

                       
                       
                       
                       
#endif