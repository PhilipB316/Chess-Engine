// movefinder.c

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "movefinder.h"
#include "lookuptables.h"
#include "board.h"
#include "memory.h"
#include "../search/evaluate.h"

#define KINGSIDE 0
#define QUEENSIDE 1

static uint64_t num_new_positions = 0;

Position_t *POSITION;
bool WHITE_TO_MOVE;
int PIECE_COLOUR;

void generate_new_position(MoveType_t piece, ULL possible_moves_bitboard, ULL from_square_bitboard, ULL special_flags)
{
    while (possible_moves_bitboard)
    {
        // --- useful bitboards and squares ---
        register ULL to_square_bitboard = 1ULL << __builtin_ctzll(possible_moves_bitboard);
        register ULL move_bitboard = from_square_bitboard | to_square_bitboard;

        // --- allocating memory for the new position and updating parent ---
        // Position_t *new_position = malloc(sizeof(Position_t));
        Position_t *new_position = custom_alloc();
        memcpy(new_position, POSITION, sizeof(Position_t));
        num_new_positions++;
        POSITION->child_positions[POSITION->num_children++] = new_position;

        // --- setting active and opponent pieces ---
        PiecesOneColour_t *active_pieces_set = &new_position->pieces[WHITE_TO_MOVE];
        PiecesOneColour_t *opponent_pieces_set = &new_position->pieces[!WHITE_TO_MOVE];

        // --- updating the general position ---
        new_position->all_pieces ^= move_bitboard;
        new_position->white_to_move = !WHITE_TO_MOVE;
        new_position->half_move_count++;
        new_position->num_children = 0;
        new_position->en_passant_bitboard = 0;
        active_pieces_set->all_pieces ^= move_bitboard;

        // --- updating the moved piece ---
        switch (piece)
        {
        case PAWN:
            active_pieces_set->pawns ^= move_bitboard;
            break;
        case KNIGHT:
            active_pieces_set->knights ^= move_bitboard;
            break;
        case BISHOP:
            active_pieces_set->bishops ^= move_bitboard;
            break;
        case ROOK:
            active_pieces_set->rooks ^= move_bitboard;
            break;
        case DOUBLE_PUSH:
            active_pieces_set->pawns ^= move_bitboard;
            new_position->en_passant_bitboard = special_flags;
            break;
        case QUEEN:
            active_pieces_set->queens ^= move_bitboard;
            break;
        case KING:
            active_pieces_set->kings ^= move_bitboard;
            active_pieces_set->castle_kingside = false;
            active_pieces_set->castle_queenside = false;
            break;
        case CASTLE_KINGSIDE:
            active_pieces_set->kings ^= move_bitboard;
            active_pieces_set->rooks ^= rook_castling_array[WHITE_TO_MOVE][KINGSIDE];
            new_position->all_pieces ^= rook_castling_array[WHITE_TO_MOVE][KINGSIDE];
            active_pieces_set->castle_kingside = false;
            active_pieces_set->castle_queenside = false;
            break;
        case CASTLE_QUEENSIDE:
            active_pieces_set->kings ^= move_bitboard;
            active_pieces_set->rooks ^= rook_castling_array[WHITE_TO_MOVE][QUEENSIDE];
            new_position->all_pieces ^= rook_castling_array[WHITE_TO_MOVE][QUEENSIDE];
            active_pieces_set->castle_kingside = false;
            active_pieces_set->castle_queenside = false;
            break;
        case EN_PASSANT_CAPTURE:
            active_pieces_set->pawns ^= move_bitboard;
            // remove the captured pawn
            opponent_pieces_set->pawns ^= special_flags;
            opponent_pieces_set->all_pieces ^= special_flags;
            new_position->all_pieces ^= special_flags;
            new_position->piece_value_diff += PIECE_COLOUR * PAWN_VALUE;
            break;
        case PROMOTE_QUEEN:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->queens |= to_square_bitboard;
            new_position->piece_value_diff += PIECE_COLOUR * (QUEEN_VALUE - PAWN_VALUE);
            break;
        case PROMOTE_ROOK:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->rooks |= to_square_bitboard;
            new_position->piece_value_diff += PIECE_COLOUR * (ROOK_VALUE - PAWN_VALUE);
            break;
        case PROMOTE_BISHOP:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->bishops |= to_square_bitboard;
            new_position->piece_value_diff += PIECE_COLOUR * (BISHOP_VALUE - PAWN_VALUE);
            break;
        case PROMOTE_KNIGHT:
            active_pieces_set->pawns &= ~from_square_bitboard;
            active_pieces_set->knights |= to_square_bitboard;
            new_position->piece_value_diff += PIECE_COLOUR * (KNIGHT_VALUE - PAWN_VALUE);
            break;
        }

        // --- updating the opponent pieces if captures ---
        if (opponent_pieces_set->all_pieces & to_square_bitboard)
        {
            opponent_pieces_set->all_pieces ^= to_square_bitboard;
            if (opponent_pieces_set->pawns & to_square_bitboard)
            {
                opponent_pieces_set->pawns ^= to_square_bitboard;
                new_position->piece_value_diff += PIECE_COLOUR * PAWN_VALUE;
            }
            else if (opponent_pieces_set->knights & to_square_bitboard)
            {
                opponent_pieces_set->knights ^= to_square_bitboard;
                new_position->piece_value_diff += PIECE_COLOUR * KNIGHT_VALUE;
            }
            else if (opponent_pieces_set->bishops & to_square_bitboard)
            {
                opponent_pieces_set->bishops ^= to_square_bitboard;
                new_position->piece_value_diff += PIECE_COLOUR * BISHOP_VALUE;
            }
            else if (opponent_pieces_set->rooks & to_square_bitboard)
            {
                opponent_pieces_set->rooks ^= to_square_bitboard;
                new_position->piece_value_diff += PIECE_COLOUR * ROOK_VALUE;
            }
            else if (opponent_pieces_set->queens & to_square_bitboard)
            {
                opponent_pieces_set->queens ^= to_square_bitboard;
                new_position->piece_value_diff += PIECE_COLOUR * QUEEN_VALUE;
            }
            else
            {
                opponent_pieces_set->kings ^= to_square_bitboard;
                new_position->piece_value_diff += PIECE_COLOUR * KING_VALUE;
            }
        }

        possible_moves_bitboard &= ~to_square_bitboard;
    }
}

void move_finder(Position_t *position)
{
    int64_t value = position->piece_value_diff;
    // if position is checkmate, do not generate moves
    if (value < -CHECKMATE_VALUE || value > CHECKMATE_VALUE)
    {
        return;
    }

    POSITION = position;
    WHITE_TO_MOVE = position->white_to_move;
    ULL all_pieces_bitboard = position->all_pieces;
    ULL opponent_pieces_bitboard = position->pieces[!WHITE_TO_MOVE].all_pieces;
    PiecesOneColour_t *active_pieces_set = &position->pieces[WHITE_TO_MOVE];
    PiecesOneColour_t *opponent_pieces_set = &position->pieces[!WHITE_TO_MOVE];
    uint8_t start_rank, seventh_rank, en_passant_rank;
    int direction;


    if (WHITE_TO_MOVE)
    {
        direction = -1;
        start_rank = 6;
        seventh_rank = 1;
        en_passant_rank = 3;
        PIECE_COLOUR = WHITE_PIECE_COLOUR;
    }
    else
    {
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
        generate_new_position(QUEEN, possible_move_squares, from_square_bitboard, 0);
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
        generate_new_position(ROOK, possible_move_squares, from_square_bitboard, 0);
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
        generate_new_position(BISHOP, possible_move_squares, from_square_bitboard, 0);
        bishop_bitboard &= ~from_square_bitboard;
    }

    // ------------------------------- KNIGHT MOVES -------------------------------
    while (knight_bitboard)
    {
        from_square = __builtin_ctzll(knight_bitboard);
        from_square_bitboard = 1ULL << from_square;
        possible_move_squares = knight_attack_lookup_table[from_square] & ~active_pieces_bitboard;
        generate_new_position(KNIGHT, possible_move_squares, from_square_bitboard, 0);
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
        if (single_push_bitboard & ~all_pieces_bitboard)
        {
            possible_move_squares |= single_push_bitboard;

            // double push
            ULL double_push_bitboard = 1ULL << (from_square + direction * 16);
            if ((rank == start_rank) && (double_push_bitboard & ~all_pieces_bitboard))
            {
                generate_new_position(DOUBLE_PUSH, double_push_bitboard, from_square_bitboard, single_push_bitboard);
            }
        }

        if (rank != seventh_rank)
        {
            // if move is standard (not promoting)
            generate_new_position(PAWN, possible_move_squares, from_square_bitboard, 0);
        }
        else
        {
            // generate other possible moves if not promoting
            while (possible_move_squares)
            {
                to_square_bitboard = 1ULL << __builtin_ctzll(possible_move_squares);
                generate_new_position(PROMOTE_QUEEN, to_square_bitboard, from_square_bitboard, 0);
                generate_new_position(PROMOTE_ROOK, to_square_bitboard, from_square_bitboard, 0);
                generate_new_position(PROMOTE_BISHOP, to_square_bitboard, from_square_bitboard, 0);
                generate_new_position(PROMOTE_KNIGHT, to_square_bitboard, from_square_bitboard, 0);
                possible_move_squares &= ~to_square_bitboard;
            }
        }

        // check for possible en passant captures
        if ((rank == en_passant_rank) && (possible_attacks_bitboard & POSITION->en_passant_bitboard))
        {
            ULL en_passant_bitboard = POSITION->en_passant_bitboard;
            uint8_t en_passant_moved_square = __builtin_ctzll(en_passant_bitboard) - (direction * 8);
            ULL pawn_to_capture_bitboard = 1ULL << en_passant_moved_square;
            generate_new_position(EN_PASSANT_CAPTURE, en_passant_bitboard, from_square_bitboard, pawn_to_capture_bitboard);
        }

        pawn_bitboard &= ~(from_square_bitboard);
    }

    // ------------------------------- KING MOVES -------------------------------
    ULL threatened_squares = 0;
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

    threatened_squares |= king_attack_lookup_table[__builtin_ctzll(opponent_pieces_set->kings)];
    possible_move_squares = king_attack_lookup_table[__builtin_ctzll(king_bitboard)] & ~threatened_squares & ~(active_pieces_set->all_pieces);
    generate_new_position(KING, possible_move_squares, king_bitboard, 0);

    // castling kingside
    if (active_pieces_set->castle_kingside)
    {
        ULL must_be_empty = (threatened_squares | all_pieces_bitboard) & castling_blocker_masks[WHITE_TO_MOVE][KINGSIDE];
        if (!must_be_empty)
        {
            generate_new_position(CASTLE_KINGSIDE, king_castling_array[WHITE_TO_MOVE][KINGSIDE], king_bitboard, 0);
        }
    }

    // castling queenside
    if (active_pieces_set->castle_queenside)
    {
        ULL must_be_empty = (threatened_squares | all_pieces_bitboard) & castling_blocker_masks[WHITE_TO_MOVE][QUEENSIDE];
        if (!must_be_empty)
        {
            generate_new_position(CASTLE_QUEENSIDE, king_castling_array[WHITE_TO_MOVE][QUEENSIDE], king_bitboard, 0);
        }
    }
}

uint64_t get_num_new_positions(void)
{
    return num_new_positions;
}

void move_finder_init(void)
{
    generate_lookup_tables();
    if (DEBUG)
    {
        printf("---lookup-tables-generated---\n");
    }
}

void depth_move_finder(Position_t* position, uint8_t depth)
{
    if (depth == 0)
    {
        return;
    }
    move_finder(position);
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        depth_move_finder(position->child_positions[i], depth - 1);
    }
}

void free_children_memory(Position_t *position)
{
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        // free(position->child_positions[i]);
        custom_free();
    }
}

void free_depth_memory(Position_t* position, uint8_t depth)
{
    if (depth == 0)
    {
        return;
    }
    for (uint8_t i = 0; i < position->num_children; i++)
    {
        free_depth_memory(position->child_positions[i], depth - 1);
    }
    // free(position);
    custom_free();
}

