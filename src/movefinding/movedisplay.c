// File: movedisplay.c

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "movefinder.h"

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

typedef enum {
    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_ROOK,
    PIECE_QUEEN,
    PIECE_KING,
} PieceType_t;

int parse_move_notation(char *move_notation, 
                        MoveType_t *move_type,
                        uint8_t* to_square, 
                        char *disambiguation, 
                        bool *is_capture);

ULL determine_from_square_bitboard(Position_t *position, 
                                   MoveType_t move_type, 
                                   uint8_t to_square, 
                                   char *disambiguation);

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
        default: return -1; // Invalid move type
    }
}

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


void make_move_from_notation(char *move_notation, Position_t *source, Position_t *destination) 
{
    MoveType_t move_type;
    uint8_t to_square = 0; // This will be set based on the move notation
    char disambiguation[2] = "\0"; // This will be set based on the move notation
    bool is_capture = false; // This will be set based on the move notation

    parse_move_notation(move_notation, &move_type, &to_square, disambiguation, &is_capture);
    ULL from_square_bitboard = determine_from_square_bitboard(source, move_type, to_square, disambiguation);
}

int parse_move_notation(char *move_notation, 
                        MoveType_t *move_type,
                        uint8_t* to_square, 
                        char *disambiguation, 
                        bool *is_capture)
{
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
                return 1; // kingside castling
            } else if (length == 5) {
                *move_type = CASTLE_QUEENSIDE;
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
        *move_type = PAWN;
        if (length == 4) {
            disambiguation[0] = move_notation[0];
        }
    }
    return 0;
}


ULL determine_from_square_bitboard(Position_t *position, 
                                   MoveType_t move_type, 
                                   uint8_t to_square, 
                                   char *disambiguation) 
{
    PieceType_t piece_type = get_piece_type_from_move_type(move_type);
    ULL relevant_bitboard = 0;
    uint8_t from_square = 0;
    ULL disambiguation_mask = filter_disambiguation(disambiguation);

    switch (piece_type) {

        case PIECE_PAWN:
            relevant_bitboard = position->pieces[WHITE_INDEX].pawns & disambiguation_mask;
            while (relevant_bitboard) {
                from_square = __builtin_ctzll(relevant_bitboard);
                ULL pawn_moves = find_pawn_moves(position, from_square);
                if (pawn_moves & (1ULL << to_square)) {
                    return 1ULL << from_square;
                }
                relevant_bitboard &= ~(1ULL << from_square);
            }
            break;

        case PIECE_KNIGHT:
            relevant_bitboard = position->pieces[WHITE_INDEX].knights & disambiguation_mask;
            while (relevant_bitboard) {
                from_square = __builtin_ctzll(relevant_bitboard);
                ULL knight_moves = find_knight_moves(position, from_square);
                if (knight_moves & (1ULL << to_square)) {
                    return 1ULL << from_square;
                }
                relevant_bitboard &= ~(1ULL << from_square);
            }
            break;

        case PIECE_BISHOP:
            relevant_bitboard = position->pieces[WHITE_INDEX].bishops & disambiguation_mask;
            while (relevant_bitboard) {
                from_square = __builtin_ctzll(relevant_bitboard);
                ULL bishop_moves = find_bishop_moves(position, from_square);
                if (bishop_moves & (1ULL << to_square)) {
                    return 1ULL << from_square;
                }
                relevant_bitboard &= ~(1ULL << from_square);
            }
            break;

        case PIECE_ROOK:
            relevant_bitboard = position->pieces[WHITE_INDEX].rooks & disambiguation_mask;
            while (relevant_bitboard) {
                from_square = __builtin_ctzll(relevant_bitboard);
                ULL rook_moves = find_rook_moves(position, from_square);
                if (rook_moves & (1ULL << to_square)) {
                    return 1ULL << from_square;
                }
                relevant_bitboard &= ~(1ULL << from_square);
            }
            break;

        case PIECE_QUEEN:
            relevant_bitboard = position->pieces[WHITE_INDEX].queens & disambiguation_mask;
            while (relevant_bitboard) {
                from_square = __builtin_ctzll(relevant_bitboard);
                ULL queen_moves = find_queen_moves(position, from_square);
                if (queen_moves & (1ULL << to_square)) {
                    return 1ULL << from_square;
                }
                relevant_bitboard &= ~(1ULL << from_square);
            }
            break;

        case PIECE_KING:
            // since there is only only one king it must be the only piece on the board
            return position->pieces[WHITE_INDEX].kings;
    }
    return 0; // No valid from square found
}






















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
int find_source_square(PiecesOneColour_t *parent, PiecesOneColour_t *child, int piece_type, int to_square) {
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
    free_children_memory(position);
    return found_count;
}

int find_from_square(Position_t *position, MoveType_t move_type, uint8_t to_square, char *disambiguation) 
{
    int from_squares[8];
    int num_from_squares = 0;

    // Determine the piece type based on the move type
    int piece_type = 0; // Default to pawn
    switch (move_type) {
        case KNIGHT: piece_type = 1; break;
        case BISHOP: piece_type = 2; break;
        case ROOK: piece_type = 3; break;
        case QUEEN: piece_type = 4; break;
        case KING: piece_type = 5; break;
        default: break; // Pawn or other types don't need special handling
    }

    num_from_squares = get_legal_moves(position, piece_type, to_square, from_squares, 8);

    if (num_from_squares == 0) return -1; // No valid moves found

    // If disambiguation is provided, find the correct source square
    if (disambiguation[0] != '\0') {
        for (int i = 0; i < num_from_squares; ++i) {
            if ((from_squares[i] % 8) == (disambiguation[0] - 'a')) { // Check file
                return from_squares[i];
            }
        }
    }

    // If no disambiguation, return the first found square
    return from_squares[0];
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

