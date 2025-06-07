// evaluate.c

#include <stdint.h>
#include <stdio.h>

#include "../movefinding/board.h"
#include "../movefinding/lookuptables.h"
#include "evaluate.h"

static uint16_t half_move_count = 0;

int64_t opening_evaluation(Position_t* position);
int64_t midgame_evaluation(Position_t* position);
ULL calculate_attack_squares(Position_t* position);

int64_t evaluate_position(Position_t* position)
{
    if (half_move_count < MID_GAME_MOVE_COUNT)
    {
        return opening_evaluation(position);
    } else {
        return midgame_evaluation(position);
    }
}

int64_t opening_evaluation(Position_t* position)
{
    /**
    * TODO: account for multiple attacks on same square
    */
    int64_t score = position->piece_value_diff * (position->white_to_move ? 1 : -1);
    ULL attack_squares = calculate_attack_squares(position);
    score += __builtin_popcountll(attack_squares & CENTER_FOUR_SQUARES) * CENTER_SQUARE_ATTACK_VALUE;
    score += __builtin_popcountll(attack_squares & BOX_SQUARES) * BOX_SQUARE_ATTACK_VALUE;
    score += __builtin_popcountll(attack_squares) * GENERAL_ATTACK_SQUARES_VALUE;
    return score;
}

int64_t midgame_evaluation(Position_t* position)
{
    int64_t score = position->piece_value_diff * (position->white_to_move ? 1 : -1);
    ULL attack_squares = calculate_attack_squares(position);
    score += __builtin_popcountll(attack_squares) * GENERAL_ATTACK_SQUARES_VALUE;
    return score;
}


ULL calculate_attack_squares(Position_t* position)
{
    PiecesOneColour_t *active_pieces_set = &position->pieces[position->white_to_move];
    uint8_t from_square;
    uint16_t index;
    ULL from_square_bitboard, threatened_squares = 0;
    ULL all_pieces_bitboard = position->all_pieces;

    // ============================== PAWNS ==============================
    ULL pawn_bitboard = active_pieces_set->pawns;
    while (pawn_bitboard)
    {
        from_square = __builtin_ctzll(pawn_bitboard);
        threatened_squares |= pawn_attack_lookup_table[position->white_to_move][from_square];
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

    ULL rook_blockers, bishop_blockers;

    // ============================== BISHOPS ==============================
    ULL rook_bitboard = active_pieces_set->rooks;
    while (rook_bitboard)
    {
        from_square = __builtin_ctzll(rook_bitboard);
        from_square_bitboard = 1ULL << from_square;
        rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        threatened_squares |= rook_attack_lookup_table[from_square][index];
        rook_bitboard &= ~from_square_bitboard;
    }

    // ============================== ROOKS ==============================
    ULL bishop_bitboard = active_pieces_set->bishops;
    while (bishop_bitboard)
    {
        from_square = __builtin_ctzll(bishop_bitboard);
        from_square_bitboard = 1ULL << from_square;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        bishop_bitboard &= ~from_square_bitboard;
    }

    // ============================== QUEENS ==============================
    ULL queen_bitboard = active_pieces_set->queens;
    while (queen_bitboard)
    {
        from_square = __builtin_ctzll(queen_bitboard);
        from_square_bitboard = 1ULL << from_square;
        rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        threatened_squares |= rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        queen_bitboard &= ~from_square_bitboard;
    }
    
    // ============================== KINGS ==============================
    from_square = __builtin_ctzll(active_pieces_set->kings);
    threatened_squares |= king_attack_lookup_table[from_square];

    return threatened_squares;
}

void set_half_move_count(uint16_t count)
{
    half_move_count = count;
}

