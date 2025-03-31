/**
 * @file movefinder.c
 * @brief This file implements move generation
 * @author Philip Brand
 * @date 2024-12-06
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"
#include "move_lookuptables.h"
#include "board_formatter.h"

// global variables to allow all functions to access pointers to important arrays/ information
static Position_t *POSITION_LIST;
static Position_t *POSITION;
static size_t *NUM_POSITIONS;
static bool WHITE_TO_MOVE;
static PiecesOneColour_t *OPPONENT_PIECES;

void move_finder(Position_t *position_list,
                 size_t *num_positions,
                 Position_t *const position,
                 PiecesOneColour_t *active_pieces,
                 PiecesOneColour_t *opponent_pieces)
{
    ULL all_pieces = position->all_pieces;
    ULL rook_blockers, bishop_blockers;
    uint8_t from_square;
    ULL possible_move_squares = 0;
    uint16_t index;
    ULL not_active_pieces = ~active_pieces->all_pieces;

    WHITE_TO_MOVE = position->white_to_move;
    POSITION_LIST = position_list;
    NUM_POSITIONS = num_positions;
    OPPONENT_PIECES = opponent_pieces;
    POSITION = position;

    uint8_t start_rank, seventh_rank;
    int direction;

    if (WHITE_TO_MOVE)
    {
        start_rank = 6;
        seventh_rank = 1;
        direction = -1;
    }
    else
    {
        start_rank = 1;
        seventh_rank = 6;
        direction = 1;
    }

    // ========================= QUEENS =========================
    ULL queen_bitboard = active_pieces->queens;
    while (queen_bitboard)
    {
        from_square = __builtin_ctzll(queen_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_move_squares |= rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_move_squares |= bishop_attack_lookup_table[from_square][index];
        possible_move_squares &= not_active_pieces;
        queen_bitboard &= ~(1ULL << (from_square));
    }

    // ========================= ROOKS =========================
    ULL rook_bitboard = active_pieces->rooks;
    while (rook_bitboard)
    {
        from_square = __builtin_ctzll(rook_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_move_squares = rook_attack_lookup_table[from_square][index];
        possible_move_squares &= not_active_pieces;
        // generate_position_from_bitboard(ROOK, from_square, possible_moves, false);
        rook_bitboard &= ~(1ULL << (from_square));
    }

    // ========================= BISHOPS =========================
    ULL bishop_bitboard = active_pieces->bishops;
    while (bishop_bitboard)
    {
        from_square = __builtin_ctzll(bishop_bitboard);
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_move_squares = bishop_attack_lookup_table[from_square][index];
        possible_move_squares &= not_active_pieces;
        // generate_position_from_bitboard(BISHOP, from_square, possible_moves, false);
        bishop_bitboard &= ~(1ULL << (from_square));
    }

    // ========================== KNIGHTS =========================
    ULL knight_bitboard = active_pieces->knights;
    while (knight_bitboard)
    {
        from_square = __builtin_ctzll(knight_bitboard);
        possible_move_squares = knight_attack_lookup_table[from_square];
        possible_move_squares &= not_active_pieces;
        // generate_position_from_bitboard(KNIGHT, from_square, possible_moves, false);
        knight_bitboard &= ~(1ULL << from_square);
    }

    // ========================== PAWNS =========================
    ULL pawn_bitboard = active_pieces->pawns;
    ULL possible_promotions, capture_squares;
    ULL en_passant_square = 1ULL << position->en_passant;
    while (pawn_bitboard)
    {
        from_square = __builtin_ctzll(pawn_bitboard);
        uint8_t rank = from_square / 8;
        // possible pawn captures from lookup table:
        ULL attack_squares = pawn_attack_lookup_table[WHITE_TO_MOVE][from_square];
        possible_move_squares = attack_squares & (OPPONENT_PIECES->all_pieces);
        // square number for single pushes - used for avoiding repeating calculations
        uint8_t single_push_square = from_square + direction * 8;
        ULL single_push = 1ULL << (single_push_square);

        // single pushes (normal)
        if (single_push & ~all_pieces)
        {
            possible_move_squares |= single_push;
            // if single moves then possibly double moves:
            ULL double_push_square = 1ULL << (from_square + direction * 16);
            if ((rank == start_rank) && (double_push_square & ~all_pieces))
            {
                // if double moves then set special flag to possible en passant square, and generate move
                // generate_position_from_bitboard(PAWN, from_square, double_push_square, single_push_square);
            }
        }
        // generate other possible moves if not promoting
        if (rank != seventh_rank)
        {
            // generate_position_from_bitboard(PAWN, from_square, possible_moves, 0);
        }
        else
        {
            // and if promoting:
            // generate_position_from_bitboard(PAWN, from_square, possible_moves, 1);
        }

        if (en_passant_square & attack_squares)
        {
            uint8_t en_passent_pawn_square = position->en_passant - direction * 8;
            // generate_position_from_bitboard(PAWN, from_square, en_passant_square, 100 + en_passent_pawn_square);
        }
        pawn_bitboard &= ~(1ULL << from_square);
    }

    // ========================== KINGS =========================
    ULL king_bitboard = active_pieces->kings;
    ULL threatened_squares = 0;

    ULL opponent_diagonals_bitboard = opponent_pieces->queens | opponent_pieces->bishops;
    while (opponent_diagonals_bitboard)
    {
        from_square = __builtin_ctzll(opponent_diagonals_bitboard);
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = bishop_blockers * actual_bishop_magic_numbers[from_square] >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        opponent_diagonals_bitboard &= ~(1ULL << from_square);
    }
    ULL opponent_straights_bitboard = opponent_pieces->queens | opponent_pieces->rooks;
    while (opponent_straights_bitboard)
    {
        from_square = __builtin_ctzll(opponent_straights_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        index = rook_blockers * actual_rook_magic_numbers[from_square] >> offset_RBits[from_square];
        threatened_squares |= rook_attack_lookup_table[from_square][index];
        opponent_straights_bitboard &= ~(1ULL << from_square);
    }
    ULL opponent_knights = opponent_pieces->knights;
    while (opponent_knights)
    {
        from_square = __builtin_ctzll(opponent_knights);
        threatened_squares |= knight_attack_lookup_table[from_square];
        opponent_knights &= ~(1ULL << from_square);
    }
    ULL opponent_pawns = opponent_pieces->pawns;
    while (opponent_pawns)
    {
        from_square = __builtin_ctzll(opponent_pawns);
        threatened_squares |= pawn_attack_lookup_table[!WHITE_TO_MOVE][from_square];
        opponent_pawns &= ~(1ULL << from_square);
    }
    from_square = __builtin_ctzll(opponent_pieces->kings);
    threatened_squares |= king_attack_lookup_table[from_square];
    // print_bitboard(threatened_squares);

    from_square = __builtin_ctzll(king_bitboard);
    possible_move_squares = king_attack_lookup_table[from_square] & not_active_pieces & ~threatened_squares;
    // generate_position_from_bitboard(KING, from_square, possible_moves, 0);

    ULL must_be_empty = (threatened_squares | all_pieces) & castling_blocker_masks[WHITE_TO_MOVE][0];
    if (!must_be_empty && active_pieces->castle_kingside)
    {
        // generate_position_from_bitboard(KING, from_square, 1ULL << (from_square + 2), CASTLING_KINGSIDE);
    }
    must_be_empty = (threatened_squares | all_pieces) & castling_blocker_masks[WHITE_TO_MOVE][1];
    if (!must_be_empty && active_pieces->castle_queenside)
    {
        // generate_position_from_bitboard(KING, from_square, 1ULL << (from_square - 2), CASTLING_QUEENSIDE);
    }
}

// void move_maker(Position_t* const old_position, Position_t* new_position, Move_t* move)
// {

//     new_position->all_pieces ^= bitboard_move;
//     switch (move->captured)
//     {
//         case NONE:
//             break;
//         case PAWN:
//             opponent_pieces->pawns &= ~bitboard_to_square;
//             opponent_pieces->all_pieces &= ~bitboard_to_square;
//             break;
//         case KNIGHT:
//             opponent_pieces->knights &= ~bitboard_to_square;
//             opponent_pieces->all_pieces &= ~bitboard_to_square;
//             break;
//         case BISHOP:
//             opponent_pieces->bishops &= ~bitboard_to_square;
//             opponent_pieces->all_pieces &= ~bitboard_to_square;
//             break;
//         case ROOK:
//             opponent_pieces->rooks &= ~bitboard_to_square;
//             opponent_pieces->all_pieces &= ~bitboard_to_square;
//             break;
//         case QUEEN:
//             opponent_pieces->queens &= ~bitboard_to_square;
//             opponent_pieces->all_pieces &= ~bitboard_to_square;
//             break;
//     }
//     switch (move->special_flags)
//    {
//         ULL rook_move;
//         case NORMAL_MOVE:
//             switch (move->moved)
//                 {
//                     case PAWN:
//                         active_pieces->pawns ^= bitboard_move;
//                         break;
//                     case KNIGHT:
//                         active_pieces->knights ^= bitboard_move;
//                         break;
//                     case BISHOP:
//                         active_pieces->bishops ^= bitboard_move;
//                         break;
//                     case ROOK:
//                         active_pieces->rooks ^= bitboard_move;
//                         break;
//                     case QUEEN:
//                         active_pieces->queens ^= bitboard_move;
//                         break;
//                     case KING:
//                         active_pieces->kings ^= bitboard_move;
//                         break;
//                 }
//                 active_pieces->all_pieces ^= bitboard_move;
//             break;
//         case PROMOTION:
//             active_pieces->pawns &= ~bitboard_from_square;
//             switch (move->moved)
//             {
//                 case KNIGHT:
//                     active_pieces->knights |= bitboard_to_square;
//                     break;
//                 case BISHOP:
//                     active_pieces->bishops |= bitboard_to_square;
//                     break;
//                 case ROOK:
//                     active_pieces->rooks |= bitboard_to_square;
//                     break;
//                 case QUEEN:
//                     active_pieces->queens |= bitboard_to_square;
//                     break;
//             }
//             break;

//         case CASTLING_KINGSIDE:
//             rook_move = 1ULL << (move->to_square - 1) | 1ULL << (move->to_square + 1);
//             active_pieces->kings ^= bitboard_move;
//             active_pieces->rooks ^= rook_move;
//             active_pieces->all_pieces ^= (bitboard_move | rook_move);
//             break;

//         case CASTLING_QUEENSIDE:
//             rook_move = 1ULL << (move->to_square + 1) | 1ULL << (move->to_square - 2);
//             active_pieces->kings ^= bitboard_move;
//             active_pieces->rooks ^= rook_move;
//             active_pieces->all_pieces ^= (bitboard_move | rook_move);
//             break;

//         default:
//             if (move->special_flags > 100) {
//                 ULL capture_square = ~(1ULL << (move->special_flags - 100));
//                 active_pieces->pawns ^= bitboard_move;
//                 opponent_pieces->pawns &= capture_square;
//                 opponent_pieces->all_pieces &= capture_square;
//                 new_position->all_pieces &= capture_square;
//             } else {
//                 active_pieces->pawns ^= bitboard_move;
//             }
//             active_pieces->all_pieces ^= bitboard_move;
//     }
// }

// void print_move(Move_t move, bool is_white)
// {
//     WHITE_TO_MOVE = is_white;
//     char first_letter;
//     uint8_t file = move.from_square % 8;
//     // determine first letter of move
//     switch (move.moved)
//     {
//     case NONE:
//         break;
//     case PAWN:
//         // set first letter to 0 if pawn move
//         first_letter = '0';
//         break;
//     case KNIGHT:
//         first_letter = 'N';
//         break;
//     case BISHOP:
//         first_letter = 'B';
//         break;
//     case ROOK:
//         first_letter = 'R';
//         break;
//     case QUEEN:
//         first_letter = 'Q';
//         break;
//     case KING:
//         first_letter = 'K';
//         break;
//     }

//     // determine if first letter should be uppercase
//     if (WHITE_TO_MOVE) {
//         first_letter = toupper(first_letter);
//     } else {
//         first_letter = tolower(first_letter);
//     }

//     // handle special flags
//     switch (move.special_flags)
//     {
//         case NORMAL_MOVE:
//             // print nothing if pawn move (if first letter is '0')
//             if (first_letter != '0') {
//                 printf("%c", first_letter);
//             }
//             // print x if capture
//             if (move.captured)
//             {
//                 // print file of pawn if pawn capture
//                 if (first_letter == '0')
//                 {
//                     printf("%c", 'a' + file);
//                 }
//                 printf("x");
//             }
//             printf("%s", pretty_print_moves[move.to_square]);
//             break;

//         case PROMOTION:
//             // print promotion things
//             printf("%s", pretty_print_moves[move.to_square]);
//             printf("=%c", first_letter);
//             break;
//         case CASTLING_KINGSIDE:
//             printf("0-0");
//             break;
//         case CASTLING_QUEENSIDE:
//             printf("0-0-0");
//             break;
//         default:
//             if (move.special_flags > 100) {
//                 printf("%cx", 'a' + file);
//                 printf("%s", pretty_print_moves[move.to_square]);
//             }
//             printf("%s", pretty_print_moves[move.to_square]);
//     }
//     printf(", %d\n", move.special_flags);
// }

/**
 * @brief Generates the possible moves objects for each bit in a bitboard
 *
 * @param POSITION_LIST The list of moves to be populated
 * @param NUM_POSITIONS The number of moves in the list
 * @param white_to_move boolean indicating if white is to move
 * @param OPPONENT_PIECES The bitboards of the opponent's pieces
 * @param piece The piece that is to be moved
 * @param from_square The square index of the piece
 * @param possible_moves The bitboard of possible moves
 *                 break;

 */
// void generate_moves_from_bitboard(Piece_t piece,
//                                   uint8_t from_square,
//                                   ULL possible_moves,
//                                   uint16_t special_flags)
// {
//     while (possible_moves) {
//         if (special_flags == 1) {
//             printf("special flag 1\n");
//             // generate_moves_from_bitboard(KNIGHT, from_square, possible_moves, 0);
//             // generate_moves_from_bitboard(BISHOP, from_square, possible_moves, 0);
//             // generate_moves_from_bitboard(ROOK, from_square, possible_moves, 0);
//             // generate_moves_from_bitboard(QUEEN, from_square, possible_moves, 0);
//         } else {
//             uint8_t to_square = __builtin_ctzll(possible_moves);
//             POSITION_LIST[*NUM_POSITIONS].moved = piece;
//             POSITION_LIST[*NUM_POSITIONS].from_square = from_square;
//             POSITION_LIST[*NUM_POSITIONS].to_square = to_square;
//             POSITION_LIST[*NUM_POSITIONS].special_flags = special_flags;
//             ULL square = 1ULL << to_square;
//             // capture handling
//             if (OPPONENT_PIECES->all_pieces & square) {  // global check for captures
//                 // then specifics for each piece
//                 if (OPPONENT_PIECES->pawns & square) {
//                     POSITION_LIST[*NUM_POSITIONS].captured = PAWN;
//                 } else if (OPPONENT_PIECES->knights & square) {
//                     POSITION_LIST[*NUM_POSITIONS].captured = KNIGHT;
//                 } else if (OPPONENT_PIECES->bishops & square) {
//                     POSITION_LIST[*NUM_POSITIONS].captured = BISHOP;
//                 } else if (OPPONENT_PIECES->rooks & square) {
//                     POSITION_LIST[*NUM_POSITIONS].captured = ROOK;
//                 } else if (OPPONENT_PIECES->queens & square) {
//                     POSITION_LIST[*NUM_POSITIONS].captured = QUEEN;
//                 } else if (OPPONENT_PIECES->kings & square) {
//                     POSITION_LIST[*NUM_POSITIONS].captured = KING;
//                 }
//             } else {
//                 POSITION_LIST[*NUM_POSITIONS].captured = NONE;
//             }
//             (*NUM_POSITIONS)++;
//             possible_moves &= ~square;
//         }
//     }
// }
