// evaluate.c

#include <stdint.h>

#include "../movefinding/board.h"
#include "../movefinding/lookuptables.h"
#include "../movefinding/movefinder.h"
#include "hash_tables.h"
#include "./evaluate.h"

int32_t opening_evaluation(Position_t* position);
int32_t midgame_evaluation(Position_t* position);

bool is_check(Position_t* position, bool for_white)
{
    // side to move’s king under attack?
    ULL king_bb = position->pieces[for_white].kings;
    if (!king_bb) return false;
    uint8_t king_sq = __builtin_ctzll(king_bb);

    // squares attacked by opponent
    ULL attacks = calculate_attack_squares(position, !for_white);
    return (attacks & (1ULL << king_sq));
}

KingStatus_t determine_king_status(Position_t* position, bool for_white)
{
    if (is_threefold_repetition(position)) { return THREEFOLD_REPETITION; }

    move_finder(position);

    uint16_t num_children = position->num_children;
    uint16_t legal_count = 0;

    for (uint16_t i = 0; i < num_children; i++)
    {
        Position_t* child = position->child_positions[i];
        // A move is legal if the mover’s king is not in check in the child
        bool illegal = is_check(child, for_white);
        if (!illegal) { legal_count++; }
    }

    free_children_memory(position);
    clear_children_count(position);

    if (legal_count == 0) {
        if (is_check(position, for_white)) { return CHECKMATE; }
        else { return STALEMATE; }
    }
    if (is_check(position, for_white)) { return CHECK; }
    return BORING;
}

int32_t evaluate_position(Position_t* position)
{
    uint16_t half_move_count = position->half_move_count;
    if (half_move_count < MID_GAME_MOVE_COUNT) {
        return opening_evaluation(position);
    } else {
        return  midgame_evaluation(position);
    }
}

int32_t opening_evaluation(Position_t* position)
{
    int32_t score = position->piece_value_diff * (position->white_to_move ? 1 : -1);

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
    // ULL queen_bitboard = active_pieces_set->queens;
    // threatened_squares = 0;
    // while (queen_bitboard)
    // {
    //     from_square = __builtin_ctzll(queen_bitboard);
    //     rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
    //     bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
    //     index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
    //     threatened_squares |= rook_attack_lookup_table[from_square][index];
    //     index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
    //     threatened_squares |= bishop_attack_lookup_table[from_square][index];
    //     queen_bitboard &= ~(1ULL << from_square);
    // }
    all_threathened_squards |= threatened_squares;
    score += __builtin_popcountll(threatened_squares & CENTER_FOUR_SQUARES) * CENTER_SQUARE_ATTACK_VALUE;
    score += __builtin_popcountll(threatened_squares & BOX_SQUARES) * BOX_SQUARE_ATTACK_VALUE;

    score += __builtin_popcountll(all_threathened_squards) * GENERAL_ATTACK_SQUARES_VALUE;
    return score;
}

int32_t midgame_evaluation(Position_t* position)
{
    int32_t score = position->piece_value_diff * (position->white_to_move ? 1 : -1);
    ULL attack_squares = calculate_attack_squares(position, position->white_to_move);
    score += __builtin_popcountll(attack_squares) * GENERAL_ATTACK_SQUARES_VALUE;
    return score;
}


