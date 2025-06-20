// File: movedisplay.c

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "movefinder.h"

// Helper to get bitboard for piece type
static ULL get_piece_bitboard(PiecesOneColour_t *pieces, int piece_type) {
    switch (piece_type) {
        case 0: return pieces->pawns;
        case 1: return pieces->knights;
        case 2: return pieces->bishops;
        case 3: return pieces->rooks;
        case 4: return pieces->queens;
        case 5: return pieces->kings;
        default: return 0ULL;
    }
}

// Helper to find the source square for a move of a given piece type to to_square
static int find_source_square(PiecesOneColour_t *parent, PiecesOneColour_t *child, int piece_type, int to_square) {
    uint64_t parent_bb = get_piece_bitboard(parent, piece_type);
    uint64_t child_bb = get_piece_bitboard(child, piece_type);

    // The piece must be present on to_square in child, but not in parent
    uint64_t to_mask = 1ULL << to_square;
    if (!(child_bb & to_mask)) return -1;
    if (parent_bb & to_mask) return -1;

    // The source square is the square that was occupied in parent but not in child
    uint64_t diff = parent_bb & ~child_bb;
    if (diff == 0) return -1;

    // There should be only one bit set in diff
    for (int sq = 0; sq < 64; ++sq) {
        if (diff & (1ULL << sq)) {
            return sq;
        }
    }
    return -1;
}

/**
 * Get all legal source squares for a piece type moving to a given destination square.
 * Returns the number of possible source squares (for disambiguation).
 */
int get_legal_moves(Position_t *position, int piece_type, int to_square, int *from_squares, int max_from_squares) {
    // Generate all legal moves for the current position
    position->num_children = 0; // Reset child count
    move_finder(position);
    int found_count = 0;
    int moving_side_index = position->white_to_move ? WHITE_INDEX : !WHITE_INDEX;
    for (uint16_t child_index = 0; child_index < position->num_children && found_count < max_from_squares; ++child_index) {
        Position_t *child_position = position->child_positions[child_index];
        int source_square = find_source_square(&position->pieces[moving_side_index], &child_position->pieces[moving_side_index], piece_type, to_square);
        if (source_square != -1) {
            from_squares[found_count++] = source_square;
        }
    }
    printf("Num children: %d\n", position->num_children);
    free_children_memory(position);
    return found_count;
}

void determine_move_notation(Position_t *old_position, Position_t *new_position, char *move_notation)
{
    bool white_to_move = old_position->white_to_move;
    // Find the squares that changed between the positions
    uint64_t move_bitboard = old_position->all_pieces ^ new_position->all_pieces;
    uint64_t from_square_bitboard = move_bitboard & old_position->pieces[white_to_move ? WHITE_INDEX : !WHITE_INDEX].all_pieces;
    uint64_t to_square_bitboard = move_bitboard & new_position->pieces[white_to_move ? WHITE_INDEX : !WHITE_INDEX].all_pieces;

    int from_square = -1, to_square = -1;
    if (from_square_bitboard)
        from_square = __builtin_ffsll(from_square_bitboard) - 1;
    if (to_square_bitboard)
        to_square = __builtin_ffsll(to_square_bitboard) - 1;

    // Detect castling
    if ((from_square == 4 && to_square == 6 && white_to_move == 1) || // White kingside
        (from_square == 60 && to_square == 62 && white_to_move == 0)) // Black kingside
    {
        strcpy(move_notation, "O-O");
        goto check_suffix;
    }
    if ((from_square == 4 && to_square == 2 && white_to_move == 1) || // White queenside
        (from_square == 60 && to_square == 58 && white_to_move == 0)) // Black queenside
    {
        strcpy(move_notation, "O-O-O");
        goto check_suffix;
    }

    // Determine which piece moved
    char piece_char = '\0';
    uint64_t from_piece_bitboard = from_square_bitboard;
    int moving_side_index = white_to_move ? WHITE_INDEX : !WHITE_INDEX;
    if (from_piece_bitboard & old_position->pieces[moving_side_index].pawns) piece_char = '\0';
    else if (from_piece_bitboard & old_position->pieces[moving_side_index].knights) piece_char = 'N';
    else if (from_piece_bitboard & old_position->pieces[moving_side_index].bishops) piece_char = 'B';
    else if (from_piece_bitboard & old_position->pieces[moving_side_index].rooks) piece_char = 'R';
    else if (from_piece_bitboard & old_position->pieces[moving_side_index].queens) piece_char = 'Q';
    else if (from_piece_bitboard & old_position->pieces[moving_side_index].kings) piece_char = 'K';

    // Detect capture
    bool is_capture = (to_square_bitboard & old_position->pieces[white_to_move ? !WHITE_INDEX : WHITE_INDEX].all_pieces) != 0;

    // Detect en passant (pawn moves diagonally with no capture on the destination square)
    if (piece_char == '\0' && !is_capture && (to_square != from_square + 8 && to_square != from_square - 8)) {
        is_capture = true;
    }

    // Detect promotion
    bool is_promotion = false;
    char promotion_char = '\0';
    if (piece_char == '\0' && from_square != -1 && to_square != -1) {
        int last_rank = white_to_move ? 0 : 7;
        if ((to_square / 8) == last_rank) {
            // Check which piece appeared on the promotion square
            if (to_square_bitboard & new_position->pieces[moving_side_index].queens) {
                is_promotion = true;
                promotion_char = 'Q';
            } else if (to_square_bitboard & new_position->pieces[moving_side_index].rooks) {
                is_promotion = true;
                promotion_char = 'R';
            } else if (to_square_bitboard & new_position->pieces[moving_side_index].bishops) {
                is_promotion = true;
                promotion_char = 'B';
            } else if (to_square_bitboard & new_position->pieces[moving_side_index].knights) {
                is_promotion = true;
                promotion_char = 'N';
            }
        }
    }

    // Disambiguation for pieces (if multiple pieces of the same type can move to the destination)
    char disambiguation[4] = "";
    if (piece_char != '\0') {
        int from_squares[8], num_sources = 0;
        int piece_type = (piece_char == 'N') ? 1 :
                         (piece_char == 'B') ? 2 :
                         (piece_char == 'R') ? 3 :
                         (piece_char == 'Q') ? 4 :
                         (piece_char == 'K') ? 5 : 0;
        if (piece_type) {
            num_sources = get_legal_moves(old_position, piece_type, to_square, from_squares, 8);
            if (num_sources > 1) {
                bool same_file = false, same_rank = false;
                for (int i = 0; i < num_sources; ++i) {
                    if (from_squares[i] == from_square) continue;
                    if ((from_squares[i] % 8) == (from_square % 8)) same_file = true;
                    if ((from_squares[i] / 8) == (from_square / 8)) same_rank = true;
                }
                if (!same_file)
                    snprintf(disambiguation, sizeof(disambiguation), "%c", 'a' + (from_square % 8));
                else if (!same_rank)
                    snprintf(disambiguation, sizeof(disambiguation), "%c", '1' + (from_square / 8));
                else
                    snprintf(disambiguation, sizeof(disambiguation), "%c%c", 'a' + (from_square % 8), '1' + (from_square / 8));
            }
        }
    }

    // Build the move notation string
    char from_square_str[3], to_square_str[3];
    strcpy(from_square_str, pretty_print_moves[from_square]);
    strcpy(to_square_str, pretty_print_moves[to_square]);
    char *notation_ptr = move_notation;
    if (piece_char) *notation_ptr++ = piece_char;
    strcpy(notation_ptr, disambiguation); notation_ptr += strlen(disambiguation);
    if (piece_char == '\0' && is_capture) *notation_ptr++ = from_square_str[0]; // pawn capture: file
    if (is_capture) *notation_ptr++ = 'x';
    strcpy(notation_ptr, to_square_str); notation_ptr += 2;
    if (is_promotion) {
        *notation_ptr++ = '=';
        *notation_ptr++ = promotion_char;
    }
    *notation_ptr = '\0';

check_suffix:
    // Optionally, check for check or checkmate here
    // if (is_check(new_position, !white_to_move)) strcat(move_notation, is_checkmate(new_position, !white_to_move) ? "#" : "+");
    return;
}
