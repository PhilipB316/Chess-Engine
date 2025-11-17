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

#ifdef WEB_BUILD
extern const bool web_build;
#else
extern const bool web_build;
#endif

#define MAX_CHILDREN 100
#define FEN_LENGTH 100
#define MAXIMUM_GAME_LENGTH 500

#define WHITE_INDEX 1

#define KINGSIDE 0
#define QUEENSIDE 1

#define DEBUG 1
#define SAFE 1 // safe alerts if custom memory pool has been exhausted

#define VERSION_MAJOR 2
#define VERSION_MINOR 2
#define VERSION_PATCH 0
#define DATE_STRING "2025-11-13"

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
    int32_t piece_value_diff;
    int32_t evaluation;
    ULL all_pieces;
    ULL en_passant_bitboard;
    ULL zobrist_key;
    struct Position_t* parent_position;
    struct Position_t* child_positions[MAX_CHILDREN];
    struct PastMoveListEntry_t* move_list_entry;
    PiecesOneColour_t pieces[2];
} Position_t;

/**
 *
 * @brief Enum for the different piece types moves.
 */
typedef enum 
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    DOUBLE_PUSH,
    PROMOTE_QUEEN,
    PROMOTE_ROOK,
    PROMOTE_BISHOP,
    PROMOTE_KNIGHT,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
    EN_PASSANT_CAPTURE,
} MoveType_t;

/**
 * @brief Represents the type of piece in chess.
 */
typedef enum {
    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_ROOK,
    PIECE_QUEEN,
    PIECE_KING,
} PieceType_t;

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
void fen_to_board(char* fen, Position_t *position);

/**
 * @brief Converts a position represented as bitboards to a FEN string.
 *
 * @param position The position to be converted.
 * @param fen The FEN string to be populated.
 */
void board_to_fen(Position_t* position, char* fen);

/**
 * @brief Calculates the difference in piece values between the two players.
 * @param position The position to be evaluated.
 */
int16_t calculate_piece_value_diff(Position_t *position);

/**
 * @breif Checks if two positions are different.
 *
 * @param position1 The first position to compare.
 * @param position2 The second position to compare.
 * @return true if the positions are different, false otherwise.
 */
bool is_different(Position_t* position1, Position_t* position2);

/**
 * @brief Finds from and to bitboards based on the difference between two positions.
 *
 * @param from_position The initial position.
 * @param to_position The target position.
 * @param from_bitboard Pointer to store the bitboard of pieces in the initial position.
 * @param to_bitboard Pointer to store the bitboard of pieces in the target position.
 */
void find_from_to_square(Position_t* from_position,
                         Position_t* to_position,
                         ULL* from_bitboard,
                         ULL* to_bitboard);

/**
 * @brief Finds the move type based on the from and to positions.
 *
 * @param from_position The initial position.
 * @param to_position The target position.
 * @return The type of move (e.g., PAWN, KNIGHT, etc.).
 */
MoveType_t find_move_type(Position_t* from_position,
                          Position_t* to_position);


#endif // BOARD_FORMATTER_H

