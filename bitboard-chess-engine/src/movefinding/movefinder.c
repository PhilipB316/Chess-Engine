/**
 * @file movefinder.c
 * @brief This file implements move generation
 * @author Philip Brand
 * @date 2024-12-06
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "movefinder.h"
#include "lookuptables.h"
#include "board.h"

Position_t *POSITION;

void generate_new_position(void)
{
    print_bitboard(POSITION->white_pieces.all_pieces);
    // generate a new position
}


void queen_move_finder(ULL queen_bitboard,
                       ULL all_pieces_bitboard,
                       ULL active_pieces_bitboard)
{
    while (queen_bitboard)
    {
        uint8_t from_square = __builtin_ctzll(queen_bitboard);
        ULL rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        ULL bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        uint8_t index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        ULL possible_move_squares = rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_move_squares |= bishop_attack_lookup_table[from_square][index];
        possible_move_squares &= ~active_pieces_bitboard;
        print_bitboard(possible_move_squares);
        // do something with the possible move squares
        generate_new_position();
        queen_bitboard &= ~(1ULL << (from_square));
    }
}

void rook_move_finder(ULL rook_bitboard,
                      ULL all_pieces_bitboard,
                      ULL active_pieces_bitboard)
{
    while (rook_bitboard)
    {
        uint8_t from_square = __builtin_ctzll(rook_bitboard);
        ULL rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
        uint8_t index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        ULL possible_move_squares = rook_attack_lookup_table[from_square][index];
        possible_move_squares &= ~active_pieces_bitboard;
        // do something with the possible move squares
        rook_bitboard &= ~(1ULL << (from_square));
    }
}

void bishop_move_finder(ULL bishop_bitboard,
                        ULL all_pieces_bitboard,
                        ULL active_pieces_bitboard)
{
    while (bishop_bitboard)
    {
        uint8_t from_square = __builtin_ctzll(bishop_bitboard);
        ULL bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        uint8_t index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        ULL possible_move_squares = bishop_attack_lookup_table[from_square][index];
        possible_move_squares &= ~active_pieces_bitboard;
        // do something with the possible move squares
        bishop_bitboard &= ~(1ULL << (from_square));
    }
}

void knight_move_finder(ULL knight_bitboard,
                        ULL all_pieces_bitboard,
                        ULL active_pieces_bitboard)
{
    while (knight_bitboard)
    {
        uint8_t from_square = __builtin_ctzll(knight_bitboard);
        ULL possible_move_squares = knight_attack_lookup_table[from_square];
        possible_move_squares &= ~active_pieces_bitboard;
        // do something with the possible move squares
        knight_bitboard &= ~(1ULL << (from_square));
    }
}

void pawn_move_finder(ULL pawn_bitboard,
                      Position_t *position)
{
    ULL all_pieces_bitboard = position->all_pieces;
    bool is_white_to_move = position->white_to_move;
    ULL opponent_pieces_bitboard;
    uint8_t start_rank, seventh_rank;
    int direction;

    if (is_white_to_move)
    {
        direction = -1;
        start_rank = 6;
        seventh_rank = 1;
        opponent_pieces_bitboard = position->black_pieces.all_pieces;
    }
    else
    {
        direction = 1;
        start_rank = 1;
        seventh_rank = 6;
        opponent_pieces_bitboard = position->white_pieces.all_pieces;
    }

    ULL possible_promotions, capture_squares;
    ULL en_passant_square = 1ULL << position->en_passant;
    while (pawn_bitboard)
    {
        uint8_t from_square = __builtin_ctzll(pawn_bitboard);
        uint8_t rank = from_square / 8;
        ULL attack_squares = pawn_attack_lookup_table[is_white_to_move][from_square];
        ULL possible_move_squares = attack_squares & (opponent_pieces_bitboard);
        uint8_t single_push_square = from_square + direction * 8;
        ULL single_push = 1ULL << (single_push_square);

        // single pushes (normal)
        if (single_push & ~all_pieces_bitboard)
        {
            possible_move_squares |= single_push;
            ULL double_push = 1ULL << (from_square + direction * 16);
            if ((rank == start_rank) && (double_push & ~all_pieces_bitboard))
            {
                // double push
            }
        }
        if (rank != seventh_rank)
        {
            // generate other possible moves if not promoting
        }
        else
        {
            // and if promoting
        }

        if (en_passant_square & attack_squares)
        {
            uint8_t en_passent_pawn_square = position->en_passant - direction * 8;
            // generate_position_from_bitboard(PAWN, from_square, en_passant_square, 100 + en_passent_pawn_square);
        }
        pawn_bitboard &= ~(1ULL << from_square);
    }
}

void king_move_finder(ULL all_pieces_bitboard,
                      bool is_white_to_move,
                      PiecesOneColour_t *active_pieces_set,
                      PiecesOneColour_t *opponent_pieces_set)
{
    ULL king_bitboard = active_pieces_set->kings;
    ULL threatened_squares = 0;
    uint8_t from_square, index;
    ULL bishop_blockers, rook_blockers;

    ULL opponent_diagonals_bitboard = opponent_pieces_set->queens | opponent_pieces_set->bishops;
    while (opponent_diagonals_bitboard)
    {
        from_square = __builtin_ctzll(opponent_diagonals_bitboard);
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces_bitboard;
        index = bishop_blockers * actual_bishop_magic_numbers[from_square] >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        opponent_diagonals_bitboard &= ~(1ULL << from_square);
    }
    ULL opponent_straights_bitboard = opponent_pieces_set->queens | opponent_pieces_set->rooks;
    while (opponent_straights_bitboard)
    {
        from_square = __builtin_ctzll(opponent_straights_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces_bitboard;
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
        threatened_squares |= pawn_attack_lookup_table[!is_white_to_move][from_square];
        opponent_pawns &= ~(1ULL << from_square);
    }
    from_square = __builtin_ctzll(opponent_pieces_set->kings);
    threatened_squares |= king_attack_lookup_table[from_square];

    from_square = __builtin_ctzll(king_bitboard);
    ULL possible_move_squares = king_attack_lookup_table[from_square] & opponent_pieces_set->all_pieces & ~threatened_squares;
    // captures

    ULL must_be_empty = (threatened_squares | all_pieces_bitboard) & castling_blocker_masks[is_white_to_move][0];
    if (!must_be_empty && active_pieces_set->castle_kingside)
    {
        // castle kingside
    }
    must_be_empty = (threatened_squares | all_pieces_bitboard) & castling_blocker_masks[is_white_to_move][1];
    if (!must_be_empty && active_pieces_set->castle_queenside)
    {
        // castle queenside
    }
}

void move_finder(Position_t *position)
{
    POSITION = position;
    ULL all_pieces_bitboard = position->all_pieces;
    PiecesOneColour_t *active_pieces_set, *opponent_pieces_set;
    bool is_white_to_move = position->white_to_move;

    if (is_white_to_move)
    {
        active_pieces_set = &position->white_pieces;
        opponent_pieces_set = &position->black_pieces;
    }
    else
    {
        active_pieces_set = &position->black_pieces;
        opponent_pieces_set = &position->white_pieces;
    }

    queen_move_finder(active_pieces_set->queens,
                      all_pieces_bitboard,
                      active_pieces_set->all_pieces);

    rook_move_finder(active_pieces_set->rooks,
                     all_pieces_bitboard,
                     active_pieces_set->all_pieces);

    bishop_move_finder(active_pieces_set->bishops,
                       all_pieces_bitboard,
                       active_pieces_set->all_pieces);

    knight_move_finder(active_pieces_set->knights,
                       all_pieces_bitboard,
                       opponent_pieces_set->all_pieces);

    pawn_move_finder(active_pieces_set->pawns,
                     position);

    king_move_finder(all_pieces_bitboard,
                     is_white_to_move,
                     active_pieces_set,
                     opponent_pieces_set);
}

void move_finder_init(void)
{
    generate_lookup_tables();
    printf("---lookup-tables-generated---\n");
}
