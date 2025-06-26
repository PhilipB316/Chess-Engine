/**
 * @file board_formatter.h
 * @brief Module for printing and handling chess boards.
 *
 * Provides functinality for
 * - printing bitboards
 * - printing positions
 * - converting FEN strings to positions
 * - printing children positions
 *
 * @author Philip Brand
 * @date 2025-04-01
 */

#ifndef BOARD
#define BOARD

#include <stdint.h>
#include <stdbool.h>

#define MAX_CHILDREN 100
#define FEN_LENGTH 100

#define WHITE_INDEX 1

#define DEBUG 0
#define SAFE 0 // safe alerts if custom memory pool has been exhausted

#define ULL unsigned long long

// Square names (a1, b1, ..., h8) for each square
extern char *pretty_print_moves[64];

/**
 * @brief bitboards and boolean representing a set of pieces
 */
typedef struct
{
    bool castle_kingside;
    bool castle_queenside;
    ULL pawns;
    ULL knights;
    ULL bishops;
    ULL rooks;
    ULL queens;
    ULL kings;
    ULL all_pieces;
} PiecesOneColour_t;

/**
 * @brief Represents a position in the game of chess.
 * 
 * Contains information about the pieces, their positions, whose turn it is,
 * the piece value difference, en passant square, and child positions.
 */
typedef struct Position_t
{
    bool white_to_move;
    uint16_t num_children;
    uint16_t half_move_count;
    int64_t piece_value_diff;
    ULL all_pieces;
    ULL en_passant_bitboard;
    struct Position_t* parent_position;
    struct Position_t* child_positions[MAX_CHILDREN];
    PiecesOneColour_t pieces[2];
} Position_t;

/**
 * @brief Prints the bitboard.
 *
 * @param bitboard The bitboard to be printed.
 */
void print_bitboard(uint64_t bitboard);

/**
 * @brief Converts a FEN string to a position, represented as bitboards.
 *
 * Includes castling rights, en passant square, piece values, and side to move.
 *
 * @param fen The FEN string representing the position.
 * @param position The position to be populated.
 */
void fen_to_board(char fen[], Position_t *position);

/**
 * @brief Prints the children positions of a given position.
 * @param position The position whose children are to be printed.
 */
void print_children_positions(Position_t *position);

/**
 * @brief Calculates the difference in piece values between the two players.
 * @param position The position to be evaluated.
 */
int16_t calculate_piece_value_diff(Position_t *position);

#endif // BOARD_FORMATTER_H
