// evaluate.c

#include <stdint.h>

#include "../movefinding/board.h"
#include "../movefinding/lookuptables.h"
#include "../movefinding/movefinder.h"
#include "evaluate.h"

int64_t opening_evaluation(Position_t* position);
int64_t midgame_evaluation(Position_t* position);
ULL calculate_attack_squares(Position_t* position);

bool is_check(Position_t* position)
{
    // Check if the king of the opponent is attacked by any piece of the current player
    ULL king_square = position->pieces[position->white_to_move].kings;
    position->white_to_move = !position->white_to_move; // Switch to opponent's perspective
    ULL attack_squares = calculate_attack_squares(position);
    position->white_to_move = !position->white_to_move; // Switch back to original perspective
    return (king_square & attack_squares);
}

KingStatus_t determine_king_status(Position_t* position)
{
    move_finder(position);
    uint8_t check_count = 0;
    uint16_t num_children = position->num_children;
    for (uint16_t i = 0; i < num_children; i++)
    {
        position->child_positions[i]->white_to_move = position->white_to_move; // Switch to opponent's perspective
        if (is_check(position->child_positions[i])) { check_count++; }
    }
    free_children_memory(position);
    clear_children_count(position);

    if (check_count == num_children) {
        if (is_check(position)) { return CHECKMATE; } 
        else { return STALEMATE; }
    }
    if (is_check(position)) { return CHECK; }
    return BORING;
}

int64_t evaluate_position(Position_t* position)
{
    uint16_t half_move_count = position->half_move_count;
    if (half_move_count < MID_GAME_MOVE_COUNT)
    {
        return opening_evaluation(position);
    } else {
        return midgame_evaluation(position);
    }
}

int64_t opening_evaluation(Position_t* position)
{
    int64_t score = position->piece_value_diff * (position->white_to_move ? 1 : -1);

    register PiecesOneColour_t *active_pieces_set = &position->pieces[position->white_to_move];
    register uint8_t from_square;
    register uint16_t index;
    register ULL threatened_squares = 0;
    register ULL all_threathened_squards;
    register ULL all_pieces_bitboard = position->all_pieces;

    // ============================== PAWNS ==============================
    ULL pawn_bitboard = active_pieces_set->pawns;
    while (pawn_bitboard)
    {
        from_square = __builtin_ctzll(pawn_bitboard);
        threatened_squares |= pawn_attack_lookup_table[position->white_to_move][from_square];
        pawn_bitboard &= ~(1ULL << from_square);
    }
    all_threathened_squards = threatened_squares;
    score += __builtin_popcountll(threatened_squares & CENTER_FOUR_SQUARES) * 
        (CENTER_SQUARE_ATTACK_VALUE + PAWN_CENTER_ATTACK_VALUE_OFFSET);
    score += __builtin_popcountll(threatened_squares & BOX_SQUARES) * BOX_SQUARE_ATTACK_VALUE;

    // ============================== KNIGHTS ==============================
    ULL knight_bitboard = active_pieces_set->knights;
    threatened_squares = 0;
    while (knight_bitboard)
    {
        from_square = __builtin_ctzll(knight_bitboard);
        threatened_squares |= knight_attack_lookup_table[from_square];
        knight_bitboard &= ~(1ULL << from_square);
    }
    all_threathened_squards |= threatened_squares;
    score += __builtin_popcountll(threatened_squares & CENTER_FOUR_SQUARES) * 
        (CENTER_SQUARE_ATTACK_VALUE + KNIGHT_CENTER_ATTACK_VALUE_OFFSET);
    score += __builtin_popcountll(threatened_squares & BOX_SQUARES) * BOX_SQUARE_ATTACK_VALUE;

    ULL rook_blockers, bishop_blockers;

    // ============================== BISHOPS ==============================
    ULL rook_bitboard = active_pieces_set->rooks;
    threatened_squares = 0;
    while (rook_bitboard)
    {
        from_square = __builtin_ctzll(rook_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        threatened_squares |= rook_attack_lookup_table[from_square][index];
        rook_bitboard &= ~(1ULL << from_square);
    }
    all_threathened_squards |= threatened_squares;
    score += __builtin_popcountll(threatened_squares & CENTER_FOUR_SQUARES) * CENTER_SQUARE_ATTACK_VALUE;
    score += __builtin_popcountll(threatened_squares & BOX_SQUARES) * BOX_SQUARE_ATTACK_VALUE;

    // ============================== ROOKS ==============================
    ULL bishop_bitboard = active_pieces_set->bishops;
    threatened_squares = 0;
    while (bishop_bitboard)
    {
        from_square = __builtin_ctzll(bishop_bitboard);
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        bishop_bitboard &= ~(1ULL << from_square);
    }
    all_threathened_squards |= threatened_squares;
    score += __builtin_popcountll(threatened_squares & CENTER_FOUR_SQUARES) * CENTER_SQUARE_ATTACK_VALUE;
    score += __builtin_popcountll(threatened_squares & BOX_SQUARES) * BOX_SQUARE_ATTACK_VALUE;

    // ============================== QUEENS ==============================
    ULL queen_bitboard = active_pieces_set->queens;
    threatened_squares = 0;
    while (queen_bitboard)
    {
        from_square = __builtin_ctzll(queen_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        threatened_squares |= rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        queen_bitboard &= ~(1ULL << from_square);
    }
    all_threathened_squards |= threatened_squares;
    score += __builtin_popcountll(threatened_squares & CENTER_FOUR_SQUARES) * CENTER_SQUARE_ATTACK_VALUE;
    score += __builtin_popcountll(threatened_squares & BOX_SQUARES) * BOX_SQUARE_ATTACK_VALUE;

    score += __builtin_popcountll(all_threathened_squards) * GENERAL_ATTACK_SQUARES_VALUE;
    return score;
}

int64_t midgame_evaluation(Position_t* position)
{
    int64_t score = position->piece_value_diff * (position->white_to_move ? 1 : -1);
    ULL attack_squares = calculate_attack_squares(position);
    score += __builtin_popcountll(attack_squares) * GENERAL_ATTACK_SQUARES_VALUE;
    return score;
}


