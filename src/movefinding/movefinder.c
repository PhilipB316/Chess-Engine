// movefinder.c

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lookuptables.h"
#include "movefinder.h"
#include "board.h"
#include "memory.h"
#include "../search/evaluate.h"
#include "../search/hash_tables.h"

static uint64_t num_new_positions = 0;

Position_t *OLD_POSTION;
bool WHITE_TO_MOVE;
int PIECE_COLOUR;

void populate_position(MoveType_t piece,
                       Position_t *new_position,
                       uint8_t to_square,
                       uint8_t from_square,
                       ULL to_square_bitboard,
                       ULL from_square_bitboard,
                       ULL move_bitboard,
                       ULL special_flags);

void generate_new_positions(MoveType_t piece, 
                            uint8_t from_square,
                            ULL possible_moves_bitboard,
                            ULL from_square_bitboard, 
                            ULL special_flags);

uint64_t get_num_new_positions(void)
{
    return num_new_positions;
}

void move_finder_init(void)
{
    generate_lookup_tables();
    if (DEBUG && !web_build) { printf("---lookup-tables-generated---\n"); }
}

void depth_move_finder(Position_t* position, uint8_t depth)
{
    if (depth == 0)
    { return; }
    move_finder(position);
    for (uint16_t i = 0; i < position->num_children; i++)
    { depth_move_finder(position->child_positions[i], depth - 1); }
}

void free_children_memory(Position_t *position)
{
    for (uint16_t i = 0; i < position->num_children; i++)
    {
        // free(position->child_positions[i]);
        custom_free();
    }
}

void free_depth_memory(Position_t* position, uint8_t depth)
{
    if (depth == 0) { return; }
    for (uint16_t i = 0; i < position->num_children; i++)
    { free_depth_memory(position->child_positions[i], depth - 1); } // free(position);
    custom_free();
}

void clear_grandchildren_count(Position_t *position)
{
    for (uint16_t i = 0; i < position->num_children; i++)
    { position->child_positions[i]->num_children = 0; } 
}

void clear_children_count(Position_t *position)
{
    position->num_children = 0; 
}

inline ULL find_pawn_moves(Position_t* position, uint8_t pawn_square)
{
    ULL possible_move_squares;
    bool white_to_move = position->white_to_move;
    ULL all_pieces_bitboard = position->all_pieces;
    ULL opponent_pieces_bitboard = position->pieces[!white_to_move].all_pieces;
    uint8_t start_rank, en_passant_rank;
    int direction;

    if (white_to_move) {
        direction = -1;
        start_rank = 6;
        en_passant_rank = 3;
    } else {
        direction = 1;
        start_rank = 1;
        en_passant_rank = 4;
    }
    uint8_t from_square = pawn_square;
    uint8_t rank = from_square / 8;
    ULL possible_attacks_bitboard = pawn_attack_lookup_table[white_to_move][from_square];
    possible_move_squares = possible_attacks_bitboard & opponent_pieces_bitboard;

    // single pushes (normal)
    ULL single_push_bitboard = 1ULL << (from_square + direction * 8);
    if (single_push_bitboard & ~all_pieces_bitboard)
    { possible_move_squares |= single_push_bitboard;
 
        // double push
        ULL double_push_bitboard = 1ULL << (from_square + direction * 16);
        if ((rank == start_rank) && (double_push_bitboard & ~all_pieces_bitboard)) {
            possible_move_squares |= double_push_bitboard; }
    }

    // check for possible en passant captures
    if ((rank == en_passant_rank) && (possible_attacks_bitboard & position->en_passant_bitboard)) {
        possible_move_squares |= position->en_passant_bitboard;
    }
    return possible_move_squares;
}


inline ULL find_knight_moves(Position_t *position, uint8_t knight_square)
{
    (void)position; // Unused parameter, but required for function signature
    return knight_attack_lookup_table[knight_square];
}

inline ULL find_bishop_moves(Position_t *position, uint8_t bishop_square)
{
    ULL all_pieces_bitboard = position->all_pieces;
    ULL bishop_blockers = bishop_blocker_masks[bishop_square] & all_pieces_bitboard;
    uint16_t index = (bishop_blockers * actual_bishop_magic_numbers[bishop_square]) >> offset_BBits[bishop_square];
    return bishop_attack_lookup_table[bishop_square][index];
}

inline ULL find_rook_moves(Position_t *position, uint8_t rook_square)
{
    ULL all_pieces_bitboard = position->all_pieces;
    ULL rook_blockers = rook_blocker_masks[rook_square] & all_pieces_bitboard;
    uint16_t index = (rook_blockers * actual_rook_magic_numbers[rook_square]) >> offset_RBits[rook_square];
    return rook_attack_lookup_table[rook_square][index];
}

inline ULL find_queen_moves(Position_t *position, uint8_t queen_square)
{
    ULL all_pieces_bitboard = position->all_pieces;
    ULL rook_blockers = rook_blocker_masks[queen_square] & all_pieces_bitboard;
    uint16_t index = (rook_blockers * actual_rook_magic_numbers[queen_square]) >> offset_RBits[queen_square];
    ULL possible_moves = rook_attack_lookup_table[queen_square][index];
    ULL bishop_blockers = bishop_blocker_masks[queen_square] & all_pieces_bitboard;
    index = (bishop_blockers * actual_bishop_magic_numbers[queen_square]) >> offset_BBits[queen_square];
    possible_moves |= bishop_attack_lookup_table[queen_square][index];

    return possible_moves;
}

inline ULL find_king_moves(Position_t *position, uint8_t king_square)
{
    bool white_to_move = position->white_to_move;
    (void)position; // Unused parameter, but required for function signature
    return king_attack_lookup_table[king_square] | king_castling_array[white_to_move][QUEENSIDE] | king_castling_array[white_to_move][KINGSIDE];
}

ULL calculate_attack_squares(Position_t* position, bool squares_belong_to_white)
{
    bool white_perspective = squares_belong_to_white;

    register PiecesOneColour_t *active_pieces_set = &position->pieces[white_perspective];
    register uint8_t from_square;
    register ULL threatened_squares = 0;

    // ============================== PAWNS ==============================
    ULL pawn_bitboard = active_pieces_set->pawns;
    while (pawn_bitboard)
    {
        from_square = __builtin_ctzll(pawn_bitboard);
        threatened_squares |= pawn_attack_lookup_table[white_perspective][from_square];
        pawn_bitboard &= ~(1ULL << from_square);
    }

    // ============================== KNIGHTS ==============================
    ULL knight_bitboard = active_pieces_set->knights;
    while (knight_bitboard)
    {
        from_square = __builtin_ctzll(knight_bitboard);
        threatened_squares |= knight_attack_lookup_table[from_square];
        knight_bitboard &= ~(1ULL << from_square);
    }

    // ============================== BISHOPS ==============================
    ULL rook_bitboard = active_pieces_set->rooks;
    while (rook_bitboard)
    {
        from_square = __builtin_ctzll(rook_bitboard);
        threatened_squares |= find_rook_moves(position, from_square);
        rook_bitboard &= ~(1ULL << from_square);
    }

    // ============================== ROOKS ==============================
    ULL bishop_bitboard = active_pieces_set->bishops;
    while (bishop_bitboard)
    {
        from_square = __builtin_ctzll(bishop_bitboard);
        threatened_squares |= find_bishop_moves(position, from_square);
        bishop_bitboard &= ~(1ULL << from_square);
    }

    // ============================== QUEENS ==============================
    ULL queen_bitboard = active_pieces_set->queens;
    while (queen_bitboard)
    {
        from_square = __builtin_ctzll(queen_bitboard);
        threatened_squares |= find_queen_moves(position, from_square);
        queen_bitboard &= ~(1ULL << from_square);
    }

    // ============================== KINGS ==============================
    from_square = __builtin_ctzll(active_pieces_set->kings);
    threatened_squares |= king_attack_lookup_table[from_square];
    return threatened_squares;
}

void move_finder(Position_t *position)
{
    // int32_t value = position->piece_value_diff;
    // if position is checkmate, do not generate moves
    // if (value < -CHECKMATE_VALUE || value > CHECKMATE_VALUE) { return; }

    OLD_POSTION = position;
    WHITE_TO_MOVE = position->white_to_move;
    ULL all_pieces_bitboard = position->all_pieces;
    ULL opponent_pieces_bitboard = position->pieces[!WHITE_TO_MOVE].all_pieces;
    PiecesOneColour_t *active_pieces_set = &position->pieces[WHITE_TO_MOVE];
    PiecesOneColour_t *opponent_pieces_set = &position->pieces[!WHITE_TO_MOVE];
    uint8_t start_rank, seventh_rank, en_passant_rank;
    int direction;


    if (WHITE_TO_MOVE) {
        direction = -1;
        start_rank = 6;
        seventh_rank = 1;
        en_passant_rank = 3;
        PIECE_COLOUR = WHITE_PIECE_COLOUR;
    } else {
        direction = 1;
        start_rank = 1;
        seventh_rank = 6;
        en_passant_rank = 4;
        PIECE_COLOUR = BLACK_PIECE_COLOUR;
    }

    register ULL active_pieces_bitboard = active_pieces_set->all_pieces;
    register ULL queen_bitboard = active_pieces_set->queens;
    register ULL rook_bitboard = active_pieces_set->rooks;
    register ULL bishop_bitboard = active_pieces_set->bishops;
    register ULL knight_bitboard = active_pieces_set->knights;
    register ULL pawn_bitboard = active_pieces_set->pawns;
    register ULL king_bitboard = active_pieces_set->kings;

    register uint8_t from_square;
    register ULL from_square_bitboard, to_square_bitboard, possible_move_squares;
    register ULL rook_blockers, bishop_blockers;
    register uint16_t index;

    // NOTE: movefinding for each piece type is inlined to ensure 
    // the code is as fast as possible.

    // ------------------------------- QUEEN MOVES -------------------------------
    while (queen_bitboard)
    {
        from_square = __builtin_ctzll(queen_bitboard);
        from_square_bitboard = 1ULL << from_square;
        rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_move_squares = rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_move_squares |= bishop_attack_lookup_table[from_square][index];
        possible_move_squares &= ~active_pieces_bitboard;
        generate_new_positions(QUEEN, from_square,
                               possible_move_squares, from_square_bitboard, 0);
        queen_bitboard &= ~(from_square_bitboard);
    }

    // ------------------------------- ROOK MOVES -------------------------------
    while (rook_bitboard)
    {
        from_square = __builtin_ctzll(rook_bitboard);
        from_square_bitboard = 1ULL << from_square;
        rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_move_squares = rook_attack_lookup_table[from_square][index];
        possible_move_squares &= ~active_pieces_bitboard;
        generate_new_positions(ROOK, from_square,
                               possible_move_squares, from_square_bitboard, 0);
        rook_bitboard &= ~from_square_bitboard;
    }

    // ------------------------------- BISHOP MOVES -------------------------------
    while (bishop_bitboard)
    {
        from_square = __builtin_ctzll(bishop_bitboard);
        from_square_bitboard = 1ULL << from_square;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_move_squares = bishop_attack_lookup_table[from_square][index];
        possible_move_squares &= ~active_pieces_bitboard;
        generate_new_positions(BISHOP, from_square,
                               possible_move_squares, from_square_bitboard, 0);
        bishop_bitboard &= ~from_square_bitboard;
    }

    // ------------------------------- KNIGHT MOVES -------------------------------
    while (knight_bitboard)
    {
        from_square = __builtin_ctzll(knight_bitboard);
        from_square_bitboard = 1ULL << from_square;
        possible_move_squares = knight_attack_lookup_table[from_square] & ~active_pieces_bitboard;
        generate_new_positions(KNIGHT, from_square,
                               possible_move_squares, from_square_bitboard, 0);
        knight_bitboard &= ~from_square_bitboard;
    }

    // ------------------------------- PAWN MOVES -------------------------------
    while (pawn_bitboard)
    {
        from_square = __builtin_ctzll(pawn_bitboard);
        from_square_bitboard = 1ULL << from_square;
        uint8_t rank = from_square / 8;
        ULL possible_attacks_bitboard = pawn_attack_lookup_table[WHITE_TO_MOVE][from_square];
        possible_move_squares = possible_attacks_bitboard & opponent_pieces_bitboard;

        // single pushes (normal)
        ULL single_push_bitboard = 1ULL << (from_square + direction * 8);
        if (single_push_bitboard & ~all_pieces_bitboard) {
            possible_move_squares |= single_push_bitboard;

            // double push
            ULL double_push_bitboard = 1ULL << (from_square + direction * 16);
            if ((rank == start_rank) && (double_push_bitboard & ~all_pieces_bitboard)) {
                generate_new_positions(DOUBLE_PUSH, from_square,
                                       double_push_bitboard,
                                       from_square_bitboard,
                                       single_push_bitboard);
            }
        }

        if (rank != seventh_rank) {
            // if move is standard (not promoting)
            generate_new_positions(PAWN, from_square,
                                   possible_move_squares, from_square_bitboard, 0);
        } else {
            // generate other possible moves if not promoting
            while (possible_move_squares)
            {
                to_square_bitboard = 1ULL << __builtin_ctzll(possible_move_squares);
                generate_new_positions(PROMOTE_QUEEN, from_square,
                                       to_square_bitboard, from_square_bitboard, 0);
                generate_new_positions(PROMOTE_ROOK, from_square,
                                       to_square_bitboard, from_square_bitboard, 0);
                generate_new_positions(PROMOTE_BISHOP, from_square,
                                       to_square_bitboard, from_square_bitboard, 0);
                generate_new_positions(PROMOTE_KNIGHT, from_square,
                                       to_square_bitboard, from_square_bitboard, 0);
                possible_move_squares &= ~to_square_bitboard;
            }
        }

        // check for possible en passant captures
        if ((rank == en_passant_rank) && (possible_attacks_bitboard & OLD_POSTION->en_passant_bitboard)) {
            ULL en_passant_bitboard = OLD_POSTION->en_passant_bitboard;
            uint8_t en_passant_moved_square = __builtin_ctzll(en_passant_bitboard) - (direction * 8);
            ULL pawn_to_capture_bitboard = 1ULL << en_passant_moved_square;
            generate_new_positions(EN_PASSANT_CAPTURE, from_square,
                                   en_passant_bitboard,
                                   from_square_bitboard,
                                   pawn_to_capture_bitboard);
        }

        pawn_bitboard &= ~(from_square_bitboard);
    }

    // ------------------------------- KING MOVES -------------------------------
    ULL threatened_squares = 0;
    all_pieces_bitboard ^= king_bitboard; // remove king from all pieces bitboard for attack calculations
    ULL opponent_diagonals_bitboard = opponent_pieces_set->queens | opponent_pieces_set->bishops;
    while (opponent_diagonals_bitboard)
    {
        from_square = __builtin_ctzll(opponent_diagonals_bitboard);
        ULL bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = bishop_blockers * actual_bishop_magic_numbers[from_square] >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        opponent_diagonals_bitboard &= ~(1ULL << from_square);
    }
    ULL opponent_straights_bitboard = opponent_pieces_set->queens | opponent_pieces_set->rooks;
    while (opponent_straights_bitboard)
    {
        from_square = __builtin_ctzll(opponent_straights_bitboard);
        ULL rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        index = rook_blockers * actual_rook_magic_numbers[from_square] >> offset_RBits[from_square];
        threatened_squares |= rook_attack_lookup_table[from_square][index];
        opponent_straights_bitboard &= ~(1ULL << from_square);
    }
    ULL opponent_knights = opponent_pieces_set->knights;
    while (opponent_knights)
    {
        from_square = __builtin_ctzll(opponent_knights);
        threatened_squares |= knight_attack_lookup_table[from_square];
        opponent_knights &= ~(1ULL << from_square);
    }
    ULL opponent_pawns = opponent_pieces_set->pawns;
    while (opponent_pawns)
    {
        from_square = __builtin_ctzll(opponent_pawns);
        threatened_squares |= pawn_attack_lookup_table[!WHITE_TO_MOVE][from_square];
        opponent_pawns &= ~(1ULL << from_square);
    }

    uint8_t king_from_square = __builtin_ctzll(king_bitboard);

    threatened_squares |= king_attack_lookup_table[__builtin_ctzll(opponent_pieces_set->kings)];
    possible_move_squares = king_attack_lookup_table[king_from_square] & 
        ~threatened_squares & ~(active_pieces_set->all_pieces);

    generate_new_positions(KING, king_from_square,
                           possible_move_squares, king_bitboard, 0);

    // castling kingside
    if (active_pieces_set->castle_kingside)
    {
        ULL must_be_empty = (threatened_squares | all_pieces_bitboard) & 
            castling_blocker_masks[WHITE_TO_MOVE][KINGSIDE];
        if (!must_be_empty) {
            generate_new_positions(CASTLE_KINGSIDE, king_from_square,
                                   king_castling_array[WHITE_TO_MOVE][KINGSIDE], king_bitboard, 0);
        }
    }

    // castling queenside
    if (active_pieces_set->castle_queenside) {
        ULL must_be_empty = (threatened_squares | all_pieces_bitboard) & 
            castling_blocker_masks[WHITE_TO_MOVE][QUEENSIDE];
        if (!must_be_empty) {
            generate_new_positions(CASTLE_QUEENSIDE, king_from_square,
                                   king_castling_array[WHITE_TO_MOVE][QUEENSIDE], king_bitboard, 0);
        }
    }
}

bool make_notation_move(Position_t *old_position,
                        Position_t *new_position,
                        MoveType_t piece,
                        ULL to_square_bitboard, 
                        ULL from_square_bitboard, 
                        ULL special_flags)
{
    // --- useful bitboards and squares ---
    register ULL move_bitboard = from_square_bitboard | to_square_bitboard;
    bool white_to_move = old_position->white_to_move;

    OLD_POSTION = old_position;
    WHITE_TO_MOVE = white_to_move;
    PIECE_COLOUR = white_to_move ? WHITE_PIECE_COLOUR : BLACK_PIECE_COLOUR;

    // --- allocating memory for the new position and updating parent ---
    memcpy(new_position, old_position, sizeof(Position_t));
    num_new_positions++;

    if (piece == CASTLE_KINGSIDE || piece == CASTLE_QUEENSIDE) {
        from_square_bitboard = new_position->pieces[white_to_move].kings;
        move_bitboard = king_castling_array[white_to_move]
            [piece == CASTLE_KINGSIDE ? KINGSIDE : QUEENSIDE] | from_square_bitboard;
        to_square_bitboard = move_bitboard & ~from_square_bitboard; // Get the new king position
    }
    if (piece == KING) {
        ULL attack_squares = calculate_attack_squares(new_position, !white_to_move);
        if (attack_squares & to_square_bitboard) { /*this is an illegal move */ return 0; }
    }

    uint8_t from_square = __builtin_ctzll(from_square_bitboard);
    uint8_t to_square = __builtin_ctzll(to_square_bitboard);

    populate_position(piece,
                      new_position,
                      to_square,
                      from_square,
                      to_square_bitboard,
                      from_square_bitboard,
                      move_bitboard,
                      special_flags);


    if (is_check(new_position, white_to_move)) {
        // illegal move, leaves king in check
        return 0;
    }
    return 1;
}

void generate_new_positions(MoveType_t piece, 
                            uint8_t from_square,
                            ULL possible_moves_bitboard,
                            ULL from_square_bitboard, 
                            ULL special_flags)
{
    while (possible_moves_bitboard)
    {
        // --- useful bitboards and squares ---
        uint8_t to_square = __builtin_ctzll(possible_moves_bitboard);
        register ULL to_square_bitboard = 1ULL << to_square;
        register ULL move_bitboard = from_square_bitboard | to_square_bitboard;

        Position_t *new_position = custom_alloc();
        memcpy(new_position, OLD_POSTION, sizeof(Position_t));

        // Build the child in-place
        populate_position(piece,
                          new_position,
                          to_square,
                          from_square,
                          to_square_bitboard,
                          from_square_bitboard,
                          move_bitboard,
                          special_flags);

        OLD_POSTION->child_positions[OLD_POSTION->num_children++] = new_position;
        num_new_positions++;

        possible_moves_bitboard &= ~to_square_bitboard;
    }
}

void populate_position(MoveType_t piece,
                       Position_t *new_position,
                       uint8_t to_square,
                       uint8_t from_square,
                       ULL to_square_bitboard,
                       ULL from_square_bitboard,
                       ULL move_bitboard,
                       ULL special_flags)
{

    // --- setting active and opponent pieces ---
    PiecesOneColour_t *active_pieces_set = &new_position->pieces[WHITE_TO_MOVE];
    PiecesOneColour_t *opponent_pieces_set = &new_position->pieces[!WHITE_TO_MOVE];

    // --- updating the general position ---
    new_position->all_pieces &= ~from_square_bitboard;
    new_position->all_pieces |= to_square_bitboard;
    new_position->white_to_move = !WHITE_TO_MOVE;
    new_position->half_move_count++;
    new_position->num_children = 0;
    new_position->en_passant_bitboard = 0;
    active_pieces_set->all_pieces ^= move_bitboard;

    ULL zobrist_key = new_position->zobrist_key;
    zobrist_key ^= zobrist_en_passant[__builtin_ctzll(OLD_POSTION->en_passant_bitboard)];
    zobrist_key ^= zobrist_black_to_move;

    switch (piece)
    {
        case PAWN:
            active_pieces_set->pawns ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][from_square];
            break;

        case KNIGHT:
            active_pieces_set->knights ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KNIGHT][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KNIGHT][from_square];
            break;

        case BISHOP:
            active_pieces_set->bishops ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_BISHOP][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_BISHOP][from_square];
            break;

        case ROOK:
            active_pieces_set->rooks ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_ROOK][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_ROOK][from_square];
            // if the rook is moved, it cannot castle anymore
            if (new_position->pieces[WHITE_TO_MOVE].castle_kingside ||
                new_position->pieces[WHITE_TO_MOVE].castle_queenside) {
                if (from_square_bitboard & original_rook_locations[WHITE_TO_MOVE][!QUEENSIDE]) {
                    active_pieces_set->castle_kingside = false;
                    zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][KINGSIDE];
                } else if (from_square_bitboard & 
                    original_rook_locations[WHITE_TO_MOVE][QUEENSIDE]) {
                    active_pieces_set->castle_queenside = false;
                    zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][QUEENSIDE];
                }
            }
            break;

        case DOUBLE_PUSH:
            active_pieces_set->pawns ^= move_bitboard;
            new_position->en_passant_bitboard = special_flags;
            zobrist_key ^= zobrist_en_passant[__builtin_ctzll(special_flags)];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][from_square];
            break;

        case QUEEN:
            active_pieces_set->queens ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_QUEEN][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_QUEEN][from_square];
            break;

        case KING:
            active_pieces_set->kings ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KING][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KING][from_square];
            if (new_position->pieces[WHITE_TO_MOVE].castle_kingside) {
                // if the king is moved, it cannot castle kingside anymore
                active_pieces_set->castle_kingside = false;
                zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][KINGSIDE];

            }
            if (new_position->pieces[WHITE_TO_MOVE].castle_queenside) {
                // if the king is moved, it cannot castle anymore
                active_pieces_set->castle_queenside = false;
                zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][QUEENSIDE];
            }
            break;

        case CASTLE_KINGSIDE:
            move_bitboard = king_castling_array[WHITE_TO_MOVE][KINGSIDE] | from_square_bitboard;
            active_pieces_set->kings ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KING][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KING][from_square];
            active_pieces_set->all_pieces ^= rook_castling_array[WHITE_TO_MOVE][KINGSIDE];
            active_pieces_set->rooks ^= rook_castling_array[WHITE_TO_MOVE][KINGSIDE];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_ROOK]
                [__builtin_ctzll(original_rook_locations[WHITE_TO_MOVE][KINGSIDE])];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_ROOK]
                [__builtin_ctzll(castled_rook_locations[WHITE_TO_MOVE][KINGSIDE])];
            new_position->all_pieces ^= rook_castling_array[WHITE_TO_MOVE][KINGSIDE];
            active_pieces_set->castle_kingside = false;
            active_pieces_set->castle_queenside = false;
            zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][KINGSIDE];
            zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][QUEENSIDE];
            break;

        case CASTLE_QUEENSIDE:
            move_bitboard = king_castling_array[WHITE_TO_MOVE][QUEENSIDE] | from_square_bitboard;
            active_pieces_set->kings ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KING][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KING][from_square];
            active_pieces_set->all_pieces ^= rook_castling_array[WHITE_TO_MOVE][QUEENSIDE];
            active_pieces_set->rooks ^= rook_castling_array[WHITE_TO_MOVE][QUEENSIDE];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_ROOK]
                [__builtin_ctzll(original_rook_locations[WHITE_TO_MOVE][QUEENSIDE])];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_ROOK]
                [__builtin_ctzll(castled_rook_locations[WHITE_TO_MOVE][QUEENSIDE])];
            new_position->all_pieces ^= rook_castling_array[WHITE_TO_MOVE][QUEENSIDE];
            active_pieces_set->castle_kingside = false;
            active_pieces_set->castle_queenside = false;
            zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][KINGSIDE];
            zobrist_key ^= zobrist_castling[WHITE_TO_MOVE][QUEENSIDE];
            break;

        case EN_PASSANT_CAPTURE:
            active_pieces_set->pawns ^= move_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][from_square];
            // remove the captured pawn
            opponent_pieces_set->pawns ^= special_flags;
            zobrist_key ^= zobrist_key_table[!WHITE_TO_MOVE][PIECE_PAWN][__builtin_ctzll(special_flags)];
            opponent_pieces_set->all_pieces ^= special_flags;
            new_position->all_pieces ^= special_flags;
            new_position->piece_value_diff += PIECE_COLOUR * PAWN_VALUE;
            break;

        case PROMOTE_QUEEN:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->queens |= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_QUEEN][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][from_square];
            new_position->piece_value_diff += PIECE_COLOUR * (QUEEN_VALUE - PAWN_VALUE);
            break;

        case PROMOTE_ROOK:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->rooks |= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_ROOK][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][from_square];
            new_position->piece_value_diff += PIECE_COLOUR * (ROOK_VALUE - PAWN_VALUE);
            break;

        case PROMOTE_BISHOP:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->bishops |= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_BISHOP][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][from_square];
            new_position->piece_value_diff += PIECE_COLOUR * (BISHOP_VALUE - PAWN_VALUE);
            break;

        case PROMOTE_KNIGHT:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->knights |= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_KNIGHT][to_square];
            zobrist_key ^= zobrist_key_table[WHITE_TO_MOVE][PIECE_PAWN][from_square];
            new_position->piece_value_diff += PIECE_COLOUR * (KNIGHT_VALUE - PAWN_VALUE);
            break;
    }

    // --- updating the opponent pieces if captures ---
    if (opponent_pieces_set->all_pieces & to_square_bitboard) {
        opponent_pieces_set->all_pieces ^= to_square_bitboard;

        if (opponent_pieces_set->pawns & to_square_bitboard) {
            opponent_pieces_set->pawns ^= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[!WHITE_TO_MOVE][PIECE_PAWN][to_square];
            new_position->piece_value_diff += PIECE_COLOUR * PAWN_VALUE;
        } else if (opponent_pieces_set->knights & to_square_bitboard) {
            opponent_pieces_set->knights ^= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[!WHITE_TO_MOVE][PIECE_KNIGHT][to_square];
            new_position->piece_value_diff += PIECE_COLOUR * KNIGHT_VALUE;
        } else if (opponent_pieces_set->bishops & to_square_bitboard) {
            opponent_pieces_set->bishops ^= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[!WHITE_TO_MOVE][PIECE_BISHOP][to_square];
            new_position->piece_value_diff += PIECE_COLOUR * BISHOP_VALUE;
        } else if (opponent_pieces_set->rooks & to_square_bitboard) {
            opponent_pieces_set->rooks ^= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[!WHITE_TO_MOVE][PIECE_ROOK][to_square];
            new_position->piece_value_diff += PIECE_COLOUR * ROOK_VALUE;
        } else if (opponent_pieces_set->queens & to_square_bitboard) {
            opponent_pieces_set->queens ^= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[!WHITE_TO_MOVE][PIECE_QUEEN][to_square];
            new_position->piece_value_diff += PIECE_COLOUR * QUEEN_VALUE;
        } else {
            opponent_pieces_set->kings ^= to_square_bitboard;
            zobrist_key ^= zobrist_key_table[!WHITE_TO_MOVE][PIECE_KING][to_square];
            new_position->piece_value_diff += PIECE_COLOUR * KING_VALUE;
        }
    }

    new_position->zobrist_key = zobrist_key;
}

