/**
 * @file movefinder.h
 * @brief movefinding
 * @author Philip Brand
 * @date 2024-12-06
 */

#ifndef MOVEFINDER_H
#define MOVEFINDER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_CHILDREN 100

#define ULL unsigned long long

/**
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
 * @brief bitboards and boolean representing a set of pieces
 */
typedef struct
{
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

/**
 * @brief Represents a position in the game of chess.
 * 
 * Contains information about the pieces, their positions, whose turn it is,
 * the piece value difference, en passant square, and child positions.
 */
typedef struct Position_t
{
    PiecesOneColour_t pieces[2];
    ULL all_pieces;
    bool white_to_move;
    int16_t piece_value_diff;
    ULL en_passant_bitboard;
    struct Position_t* parent_position;
    struct Position_t* child_positions[MAX_CHILDREN];
    uint8_t num_children;
} Position_t;

/**
 * @brief Initialises lookup tables for move finding.
 */
void move_finder_init(void);

/**
 * @brief Generates all possible moves for the current position.
 *
 * TODO: Prevent move generation for checkmated positions.
 *
 * @param position The position to generates move for.
 */
void move_finder(Position_t *position);

/**
 * @brief Free memory from allocated position and children.
 * DOES FREE THE POSITION ITSELF.
 *
 * @param position The position to free memory for.
 */
void free_children_memory(Position_t *position);

/**
    * @brief Free all memory allocated to position children.
    * DOES NOT FREE THE POSITION ITSELF.
    * 
    * @param position The position to free the children of
*/
void free_position_memory(Position_t *position);

/**
 * @brief Free all memory allocated to specific depth.
 * DOES FREE THE POSITION ITSELF.
 *
 * @param position The position to free memory for.
 * @param depth The depth of the search.
 */
void free_depth_memory(Position_t* position, uint8_t depth);

/**
 * @brief Finds all possible moves for the current position to the specified depth.
 *
 * @param position The position to generate moves for.
 * @param depth The depth of the search.
 */
void depth_move_finder(Position_t* position, uint8_t depth);

#endif
