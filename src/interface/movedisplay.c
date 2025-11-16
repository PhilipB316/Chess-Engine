// File: movedisplay.c

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "../movefinding/board.h"
#include "../movefinding/lookuptables.h"
#include "../movefinding/movefinder.h"

// rank and file masks - the entire rank or file at the given index
static ULL rank_row_masks[8] = {
    0xFF00000000000000, // Rank 8
    0x00FF000000000000, // Rank 7
    0x0000FF0000000000, // Rank 6
    0x000000FF00000000, // Rank 5
    0x00000000FF000000, // Rank 4
    0x0000000000FF0000, // Rank 3
    0x000000000000FF00, // Rank 2
    0x00000000000000FF, // Rank 1
};

static ULL file_column_masks[8] = {
    0x0101010101010101, // File a
    0x0202020202020202, // File b
    0x0404040404040404, // File c
    0x0808080808080808, // File d
    0x1010101010101010, // File e
    0x2020202020202020, // File f
    0x4040404040404040, // File g
    0x8080808080808080, // File h
};

static int square_index(char file, char rank) {
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') return -1;
    return ('8' - rank) * 8 + (file - 'a');
}

static char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

static bool is_lower(char c) {
    return (c >= 'a' && c <= 'z');
}

static bool is_numeric(char c) {
    return (c >= '0' && c <= '9');
}

static bool is_alphabetic(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * @brief Determines the simple piece type from the move type.
 *
 * @param move_type The type of the move (e.g., PAWN, KNIGHT, etc.).
 * @return The corresponding piece type.
 */
static PieceType_t get_piece_type_from_move_type(MoveType_t move_type) {
    switch (move_type) {
        case PAWN: return PIECE_PAWN;
        case KNIGHT: return PIECE_KNIGHT;
        case BISHOP: return PIECE_BISHOP;
        case ROOK: return PIECE_ROOK;
        case QUEEN: return PIECE_QUEEN;
        case KING: return PIECE_KING;
        case PROMOTE_QUEEN: return PIECE_PAWN;
        case PROMOTE_ROOK: return PIECE_PAWN;
        case PROMOTE_BISHOP: return PIECE_PAWN;
        case PROMOTE_KNIGHT: return PIECE_PAWN;
        case DOUBLE_PUSH: return PIECE_PAWN;
        case CASTLE_KINGSIDE: return PIECE_KING;
        case CASTLE_QUEENSIDE: return PIECE_KING;
        case EN_PASSANT_CAPTURE: return PIECE_PAWN;
        default: return -1; // Invalid move type
    }
}

/**
 * @brief Filters the disambiguation string to determine the bitboard for the disambiguation.
 *
 * @param disambiguation The disambiguation string (e.g., "e", "1", "a2").
 * @return The bitboard representing the disambiguation.
 */
ULL filter_disambiguation(char *disambiguation)
{
    uint8_t length = strlen(disambiguation);
    if (length == 0) {
        return ~(0x0000000000000000); // No disambiguation provided
    } else if (length == 1) {
        if (is_numeric(disambiguation[0])) {
            return rank_row_masks[disambiguation[0] - '1'];
        } else if (is_alphabetic(disambiguation[0])) {
            return file_column_masks[disambiguation[0] - 'a'];
        }
    } else {
        return 1ULL << square_index(disambiguation[0], disambiguation[1]);
    }
    return 0; // Invalid disambiguation
}

int parse_move_notation(const char *move_notation, 
                        MoveType_t *move_type,
                        uint8_t* to_square, 
                        char *disambiguation, 
                        bool *is_capture,
                        Position_t *position,
                        ULL* special_flags)
{
    bool white_to_move = position->white_to_move;
    ULL opponent_pieces_bitboard = position->pieces[!white_to_move].all_pieces;
    uint8_t length = strlen(move_notation);
    *to_square = square_index(move_notation[length - 2], move_notation[length - 1]);

    // Remove check/checkmate symbols
    if (move_notation[-1] == '#' || move_notation[-1] == '+') { length--; }
    if (length < 2 || length > 10) { return 0; }
 
    // check for captures
    for (int i = 0; i < length; ++i) {
        if (move_notation[i] == 'x') { *is_capture = true; break; }
    }

    // check for promotion
    for (int i = 0; i < length; ++i) {
        if (move_notation[i] == '=') {
            *to_square = square_index(move_notation[length - 4], move_notation[length - 3]);
            char promoted_piece = to_lower(move_notation[length - 1]);

            if (is_capture) {
                disambiguation[0] = move_notation[0];
            }
            switch (promoted_piece) {
                case 'q': *move_type = PROMOTE_QUEEN; return 1;
                case 'r': *move_type = PROMOTE_ROOK; return 1;
                case 'b': *move_type = PROMOTE_BISHOP; return 1;
                case 'n': *move_type = PROMOTE_KNIGHT; return 1;
                default: return 0; // Invalid promotion piece
            }
        }
    }

    // check for castling
    for (int i = 0; i < length; ++i) {
        if (move_notation[i] == '-') {
            if (length == 3) { *move_type = CASTLE_KINGSIDE;
                *to_square = __builtin_ctzll(king_castling_array[white_to_move][KINGSIDE]);
                return 1; // kingside castling
            } else if (length == 5) {
                *move_type = CASTLE_QUEENSIDE;
                *to_square = __builtin_ctzll(king_castling_array[white_to_move][QUEENSIDE]);
                return 1; // queenside castling
            } else {
                return 0; // Invalid castling notation
            }
        }
    }

    // check for disambiguation letters
    if (*is_capture) {
        if (move_notation[2] == 'x') {
            disambiguation[0] = move_notation[1];
        } else if(move_notation[3] == 'x') {
            disambiguation[0] = move_notation[1];
            disambiguation[1] = move_notation[2];
        }
    } else {
        if (length == 4) {
            *disambiguation = move_notation[1];
        } else if (length == 5) {
            disambiguation[0] = move_notation[1];
            disambiguation[1] = move_notation[2];
        }
    }

    char piece_type = move_notation[0];
    if (piece_type == 'K') {*move_type = KING; return 1;} // King move
    else if (piece_type == 'Q') {*move_type = QUEEN; return 1;} // Queen move
    else if (piece_type == 'R') {*move_type = ROOK; return 1;} // Rook move
    else if (piece_type == 'B') {*move_type = BISHOP; return 1;} // Bishop move
    else if (piece_type == 'N') {*move_type = KNIGHT; return 1;} // Knight move
    // Pawn move
    if (is_lower(piece_type)) {
        if (length == 4) {
            disambiguation[0] = move_notation[0];
            ULL to_square_bitboard = 1ULL << *to_square;
            if (!(opponent_pieces_bitboard & to_square_bitboard)) {
                // length 4 means a capture, and if there was no opponent piece on the to square
                // it must be an en passant capture
                *move_type = EN_PASSANT_CAPTURE; // En passant capture
                uint8_t rank_offset = (white_to_move) ? 8 : -8; // direction of pawn movement
                *special_flags = 1ULL << (*to_square + rank_offset);
                return 1;
            }
        } else if (length == 2) {
            uint8_t rank = square_index(move_notation[0], move_notation[1]) / 8;
            uint8_t double_push_rank = (white_to_move) ? 4 : 3;
            int rank_offset = (white_to_move) ? 8 : -8; // direction of pawn movement
            if (rank == double_push_rank) {
                *move_type = DOUBLE_PUSH;
                *special_flags = 1ULL << (*to_square + rank_offset);
                return 1;
            }
        }
        *move_type = PAWN;
        return 1;
    }
    return 0;
}

ULL determine_from_square_bitboard(Position_t *position,
                                   MoveType_t move_type,
                                   uint8_t to_square,
                                   char *disambiguation)
{
    PieceType_t piece_type = get_piece_type_from_move_type(move_type);
    bool white_to_move = position->white_to_move;
    ULL relevant_bitboard = 0;
    uint8_t from_square = 0;
    ULL disambiguation_mask = filter_disambiguation(disambiguation);

    // Function pointer for move generation
    ULL (*find_moves)(Position_t*, uint8_t) = NULL;

    switch (piece_type) {
        case PIECE_PAWN:
            relevant_bitboard = position->pieces[white_to_move].pawns & disambiguation_mask;
            find_moves = find_pawn_moves;
            break;
        case PIECE_KNIGHT:
            relevant_bitboard = position->pieces[white_to_move].knights & disambiguation_mask;
            find_moves = find_knight_moves;
            break;
        case PIECE_BISHOP:
            relevant_bitboard = position->pieces[white_to_move].bishops & disambiguation_mask;
            find_moves = find_bishop_moves;
            break;
        case PIECE_ROOK:
            relevant_bitboard = position->pieces[white_to_move].rooks & disambiguation_mask;
            find_moves = find_rook_moves;
            break;
        case PIECE_QUEEN:
            relevant_bitboard = position->pieces[white_to_move].queens & disambiguation_mask;
            find_moves = find_queen_moves;
            break;
        case PIECE_KING:
            relevant_bitboard = position->pieces[white_to_move].kings & disambiguation_mask;
            find_moves = find_king_moves;
            break;
        default:
            return 0;
    }

    while (relevant_bitboard) {
        from_square = __builtin_ctzll(relevant_bitboard);
        ULL moves = find_moves(position, from_square);
        if (moves & (1ULL << to_square)) {
            return 1ULL << from_square;
        }
        relevant_bitboard &= ~(1ULL << from_square);
    }
    return 0; // No valid from square found
}

bool is_ambiguous_move(Position_t* previous_position, MoveType_t move_type, uint8_t to_square)
{
    ULL piece_bitboard = 0;
    ULL (*find_moves)(Position_t*, uint8_t) = NULL;

    switch (move_type) {
        case KNIGHT:
            piece_bitboard = previous_position->pieces[previous_position->white_to_move].knights;
            find_moves = find_knight_moves;
            break;
        case BISHOP:
            piece_bitboard = previous_position->pieces[previous_position->white_to_move].bishops;
            find_moves = find_bishop_moves;
            break;
        case ROOK:
            piece_bitboard = previous_position->pieces[previous_position->white_to_move].rooks;
            find_moves = find_rook_moves;
            break;
        case QUEEN:
            piece_bitboard = previous_position->pieces[previous_position->white_to_move].queens;
            find_moves = find_queen_moves;
            break;
        default:
            return false;
    }

    uint8_t ambiguous_count = 0;
    while (piece_bitboard) {
        uint8_t from_square = __builtin_ctzll(piece_bitboard);
        ULL moves = find_moves(previous_position, from_square);
        if (moves & (1ULL << to_square)) {
            ambiguous_count++;
        }
        piece_bitboard &= ~(1ULL << from_square);
    }
    return ambiguous_count > 1;
}

void get_move_notation(Position_t* previous_position, Position_t* new_position, char* notation)
{
    MoveType_t move_type = find_move_type(previous_position, new_position);

    ULL from_bitboard, to_bitboard;
    find_from_to_square(previous_position, new_position, &from_bitboard, &to_bitboard);
    bool is_ambiguous = is_ambiguous_move(previous_position, move_type, __builtin_ctzll(to_bitboard));

    uint8_t to_square = __builtin_ctzll(to_bitboard);
    uint8_t from_square = __builtin_ctzll(from_bitboard);

    char from_square_file[2] = {'a' + (from_square % 8), '\0'};
    char from_square_rank[2] = {'1' + (7 - from_square / 8), '\0'};
    char to_square_file[2] = {'a' + (to_square % 8), '\0'};
    char to_square_rank[2] = {'1' + (7 - to_square / 8), '\0'};

    bool value_change = (new_position->piece_value_diff != previous_position->piece_value_diff);

    switch (move_type) {
        case PAWN:
            if (value_change) {
                strcat(notation, from_square_file);
                strcat(notation, "x");
            }
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case KNIGHT:
            strcat(notation, "N");
            if (is_ambiguous) {
                strcat(notation, from_square_file);
                strcat(notation, from_square_rank);
            }
            if (value_change) { strcat(notation, "x"); }
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case BISHOP:
            strcat(notation, "B");
            if (is_ambiguous) {
                strcat(notation, from_square_file);
                strcat(notation, from_square_rank);
            }
            if (value_change) { strcat(notation, "x"); }
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case ROOK:
            strcat(notation, "R");
            if (is_ambiguous) {
                strcat(notation, from_square_file);
                strcat(notation, from_square_rank);
            }
            if (value_change) { strcat(notation, "x"); }
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case QUEEN:
            strcat(notation, "Q");
            if (is_ambiguous) {
                strcat(notation, from_square_file);
                strcat(notation, from_square_rank);
            }
            if (value_change) { strcat(notation, "x"); }
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case KING:
            strcat(notation, "K");
            if (value_change) { strcat(notation, "x"); }
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case PROMOTE_QUEEN:
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            strcat(notation, "=Q");
            break;
        case PROMOTE_ROOK:
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            strcat(notation, "=R");
            break;
        case PROMOTE_BISHOP:
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            strcat(notation, "=B");
            break;
        case PROMOTE_KNIGHT:
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            strcat(notation, "=N");
            break;
        case DOUBLE_PUSH:
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case EN_PASSANT_CAPTURE:
            strcat(notation, from_square_file);
            strcat(notation, "x");
            strcat(notation, to_square_file);
            strcat(notation, to_square_rank);
            break;
        case CASTLE_KINGSIDE:
            strcat(notation, "O-O");
            break;
        case CASTLE_QUEENSIDE:
            strcat(notation, "O-O-O");
            break;
    }
}

