/**
 * @file movefinder.c
 * @brief This file implements move generation
 * @author Philip Brand
 * @date 2024-12-06
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "movefinder.h"
#include "move_lookuptables.h"

static Move_t* MOVE_LIST;
static size_t* NUM_MOVES;
static bool WHITE_TO_MOVE;
static PiecesOneColour_t* OPPONENT_PIECES;
static Position_t* POSITION;

char* pretty_print_moves[64] = 
{
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};


void print_bitboard(uint64_t bitboard)
{
    for (uint8_t i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
        {
            // pretty printing for the ranks:
            printf("\n");
            printf("%u   ", 8 - i / 8);
        }
        if (bitboard & (1ULL << i))
        {
            printf("1 ");
        }
        else
        {
            printf(". ");
        }
    }
    // pretty printing for the files:
    printf("\n\n    a b c d e f g h\n\n");
}


void print_moves(Move_t* move_list, size_t num_moves)
{
    WHITE_TO_MOVE = move_list[0].is_white;
    POSITION = move_list[0].original_position;

    for (size_t i = 1; i < num_moves; i++)
    {
        char first_letter;
        uint8_t file = move_list[i].from_square % 8;

        // determine first letter of move
        switch (move_list[i].moved)
        {
        case NONE:
            break;
        case PAWN:
            // set first letter to 0 if pawn move
            first_letter = '0';
            break;
        case KNIGHT:
            first_letter = 'N';
            break;
        case BISHOP:
            first_letter = 'B';
            break;
        case ROOK:
            first_letter = 'R';
            break;
        case QUEEN:
            first_letter = 'Q';
            break;
        case KING:
            first_letter = 'K';
            break;
        }

        // determine if first letter should be uppercase
        if (WHITE_TO_MOVE) {
            first_letter = toupper(first_letter);
        } else {
            first_letter = tolower(first_letter);
        }

        // handle special flags
        switch (move_list[i].special_flags)
        {
            case NORMAL_MOVE:
                // print nothing if pawn move (if first letter is '0')  
                if (first_letter != '0')
                {
                    printf("%s", pretty_print_moves[move_list[i].to_square]);
                    break;
                }
                // print x if capture
                if (move_list[i].captured)
                {
                    // print file of pawn if pawn capture
                    if (first_letter == '0')
                    {
                        printf("%c", 'a' + file);
                    }
                    printf("x");
                }
                printf("%s", pretty_print_moves[move_list[i].to_square]);
                break;

            case PROMOTION:
                // print promotion things
                printf("%s", pretty_print_moves[move_list[i].to_square]);
                printf("=%c", first_letter);
                break;
            case CASTLING_KINGSIDE:
                printf("0-0");
                break;
            case CASTLING_QUEENSIDE:
                printf("0-0-0");
                break;
            default:
                /**
                 * 2x possibility otherwise:
                 * if special flags is greater than 100, the move is capture.
                 * the number less 100 is the square of the pawn to be captured,
                 * therefore print as normal.
                 * otherwise, then move is a pawn double push, and the number is 
                 * the possible square to move to for capture.
                 * 
                 */
                if (move_list[i].special_flags > 100) {
                    printf("%c", 'a' + file);
                    printf("x");
                    printf("%s", pretty_print_moves[move_list[i].to_square]);
                } else {
                    printf("%s", pretty_print_moves[move_list[i].to_square]);
                }
        }

        printf(", %d\n", move_list[i].special_flags);
        // printf("Captured: %d\n", move_list[i].captured);
    }

}


void print_position(Position_t const position)
{
    char mailboxes[64] = {0};
    for (uint8_t i = 0; i < 64; i++)
    {
        if (position.white_pieces.pawns & (1ULL << i))
        {
            mailboxes[i] = 'P';
        }
        else if (position.white_pieces.knights & (1ULL << i))
        {
            mailboxes[i] = 'N';
        }
        else if (position.white_pieces.bishops & (1ULL << i))
        {
            mailboxes[i] = 'B';
        }
        else if (position.white_pieces.rooks & (1ULL << i))
        {
            mailboxes[i] = 'R';
        }
        else if (position.white_pieces.queens & (1ULL << i))
        {
            mailboxes[i] = 'Q';
        }
        else if (position.white_pieces.kings & (1ULL << i))
        {
            mailboxes[i] = 'K';
        }
        else if (position.black_pieces.pawns & (1ULL << i))
        {
            mailboxes[i] = 'p';
        }
        else if (position.black_pieces.knights & (1ULL << i))
        {
            mailboxes[i] = 'n';
        }
        else if (position.black_pieces.bishops & (1ULL << i))
        {
            mailboxes[i] = 'b';
        }
        else if (position.black_pieces.rooks & (1ULL << i))
        {
            mailboxes[i] = 'r';
        }
        else if (position.black_pieces.queens & (1ULL << i))
        {
            mailboxes[i] = 'q';
        }
        else if (position.black_pieces.kings & (1ULL << i))
        {
            mailboxes[i] = 'k';
        }
        else
        {
            mailboxes[i] = '.';
        }
    }

    for (uint8_t i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n%d   ", 8 - i / 8);
        }
        printf("%c ", mailboxes[i]);
    }
    printf("\n\n    a b c d e f g h\n");

    printf("\n");
}


Position_t fen_to_board(char* fen)
{   
    Position_t fen_position = {0};
    size_t i = 0;  // i does not necessarily count up to 64
    uint8_t square_counter = 0;  // square_counter counts up to 64
    char character = fen[i++];

    // ========================= PIECES =========================
    while (character != ' ')
    {
        PiecesOneColour_t* pieces;
        if (isalpha(character)) {
            if (isupper(character)) {
                pieces = &fen_position.white_pieces;
            } else {
                pieces = &fen_position.black_pieces;
            }
            character = tolower(character);
            if (character == 'k') {
                pieces->kings |= (1ULL << square_counter);
            } else if (character == 'q') {
                pieces->queens |= (1ULL << square_counter);
            } else if (character == 'r') {
                pieces->rooks |= (1ULL << square_counter);
            } else if (character == 'b') {
                pieces->bishops |= (1ULL << square_counter);
            } else if (character == 'n') {
                pieces->knights |= (1ULL << square_counter);
            } else if (character == 'p') {
                pieces->pawns |= (1ULL << square_counter);
            }
            pieces->all_pieces |= (1ULL << square_counter);
            fen_position.all_pieces |= (1ULL << square_counter);
            square_counter++;
        }
        else if (isdigit(character)) {
            square_counter += character - '0';
        } 
        character = fen[i++];
    }

    // ========================= WHOSE TURN =========================
    if (fen[i++] == 'w') {
        fen_position.white_to_move = true;
    } else {
        fen_position.white_to_move = false;
    }

    // ========================= CASTLING =========================
    i++;  // skip space
    character = fen[i++];
    while (character != ' ') {
        if (character == 'K') {
            fen_position.white_pieces.castle_kingside = true;
        } else if (character == 'Q') {
            fen_position.white_pieces.castle_queenside = true;
        } else if (character == 'k') {
            fen_position.black_pieces.castle_kingside = true;
        } else if (character == 'q') {
            fen_position.black_pieces.castle_queenside = true;
        }
        character = fen[i++];
    }
    // ========================= EN PASSANT =========================
    // skip space

    character = fen[i++];
    if (character != '-') {
        uint8_t file = character - 'a';
        uint8_t rank = 8 - (fen[i++] - '0');
        fen_position.en_passant = 8 * rank + file;
    } else {
        fen_position.en_passant = 0;
    }

    return fen_position;
}



/**
 * @brief Generates the possible moves objects for each bit in a bitboard
 * 
 * @param MOVE_LIST The list of moves to be populated
 * @param NUM_MOVES The number of moves in the list
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
//             // generate_moves_from_bitboard(KNIGHT, from_square, possible_moves, 0);
//             // generate_moves_from_bitboard(BISHOP, from_square, possible_moves, 0);
//             // generate_moves_from_bitboard(ROOK, from_square, possible_moves, 0);
//             // generate_moves_from_bitboard(QUEEN, from_square, possible_moves, 0);
//         } else {
//             uint8_t to_square = __builtin_ctzll(possible_moves);
//             MOVE_LIST[*NUM_MOVES].moved = piece;
//             MOVE_LIST[*NUM_MOVES].from_square = from_square;
//             MOVE_LIST[*NUM_MOVES].to_square = to_square;
//             MOVE_LIST[*NUM_MOVES].special_flags = special_flags;
//             ULL square = 1ULL << to_square;
//             // capture handling
//             if (OPPONENT_PIECES->all_pieces & square) {  // global check for captures
//                 // then specifics for each piece
//                 if (OPPONENT_PIECES->pawns & square) {
//                     MOVE_LIST[*NUM_MOVES].captured = PAWN;
//                 } else if (OPPONENT_PIECES->knights & square) {
//                     MOVE_LIST[*NUM_MOVES].captured = KNIGHT;
//                 } else if (OPPONENT_PIECES->bishops & square) {
//                     MOVE_LIST[*NUM_MOVES].captured = BISHOP;
//                 } else if (OPPONENT_PIECES->rooks & square) {
//                     MOVE_LIST[*NUM_MOVES].captured = ROOK;
//                 } else if (OPPONENT_PIECES->queens & square) {
//                     MOVE_LIST[*NUM_MOVES].captured = QUEEN;
//                 } else if (OPPONENT_PIECES->kings & square) {
//                     MOVE_LIST[*NUM_MOVES].captured = KING;
//                 }
//             } else {
//                 MOVE_LIST[*NUM_MOVES].captured = NONE;
//             }
//             (*NUM_MOVES)++;
//             possible_moves &= ~square;
//         }
//     }
// }


void generate_moves_from_bitboard(Piece_t piece,
                                  uint8_t from_square,
                                  ULL possible_moves,
                                  uint16_t special_flags)
{
    while (possible_moves) {
        uint8_t to_square = __builtin_ctzll(possible_moves);
        MOVE_LIST[*NUM_MOVES].moved = piece;
        MOVE_LIST[*NUM_MOVES].from_square = from_square;
        MOVE_LIST[*NUM_MOVES].to_square = to_square;
        MOVE_LIST[*NUM_MOVES].special_flags = special_flags;
        ULL square = 1ULL << to_square;
        // capture handling
        if (OPPONENT_PIECES->all_pieces & square) {  // global check for captures
            // then specifics for each piece
            if (OPPONENT_PIECES->pawns & square) {
                MOVE_LIST[*NUM_MOVES].captured = PAWN;
            } else if (OPPONENT_PIECES->knights & square) {
                MOVE_LIST[*NUM_MOVES].captured = KNIGHT;
            } else if (OPPONENT_PIECES->bishops & square) {
                MOVE_LIST[*NUM_MOVES].captured = BISHOP;
            } else if (OPPONENT_PIECES->rooks & square) {
                MOVE_LIST[*NUM_MOVES].captured = ROOK;
            } else if (OPPONENT_PIECES->queens & square) {
                MOVE_LIST[*NUM_MOVES].captured = QUEEN;
            } else if (OPPONENT_PIECES->kings & square) {
                MOVE_LIST[*NUM_MOVES].captured = KING;
            }
        } else {
            MOVE_LIST[*NUM_MOVES].captured = NONE;
        }

        // if condition met, the move is an en passant capture
        if (special_flags > 100) {
            MOVE_LIST[*NUM_MOVES].captured = PAWN;
        }
        (*NUM_MOVES)++;
        possible_moves &= ~square;
    }
}


void move_finder(Move_t* move_list, 
                size_t* num_moves, 
                Position_t* const position,
                PiecesOneColour_t* active_pieces,
                PiecesOneColour_t* opponent_pieces)
{
    ULL all_pieces = position->all_pieces;
    ULL rook_blockers, bishop_blockers;
    uint8_t from_square;
    ULL possible_moves = 0;
    uint16_t index;
    ULL not_active_pieces = ~active_pieces->all_pieces;

    WHITE_TO_MOVE = position->white_to_move;
    MOVE_LIST = move_list;
    NUM_MOVES = num_moves;
    OPPONENT_PIECES = opponent_pieces;

    // setting first move to store data about position
    MOVE_LIST[0].original_position = position;
    MOVE_LIST[0].is_white = WHITE_TO_MOVE;
    (*NUM_MOVES)++;

    uint8_t start_rank, seventh_rank;
    int direction;
    if (WHITE_TO_MOVE) {
        start_rank = 6;
        seventh_rank = 1;
        direction = -1;
    } else {
        start_rank = 1;
        seventh_rank = 6;
        direction = 1;
    }

    // ========================= QUEENS =========================
    ULL queen_bitboard = active_pieces->queens;
    while (queen_bitboard) {
        from_square = __builtin_ctzll(queen_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_moves |= rook_attack_lookup_table[from_square][index];
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_moves |= bishop_attack_lookup_table[from_square][index];
        possible_moves &= not_active_pieces;    
        generate_moves_from_bitboard(QUEEN, from_square, possible_moves, false);
        queen_bitboard &= ~(1ULL << (from_square));
    }

    // ========================= ROOKS =========================
    ULL rook_bitboard = active_pieces->rooks;
    while (rook_bitboard) {
        from_square = __builtin_ctzll(rook_bitboard);
        possible_moves = 0;
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        index = (rook_blockers * actual_rook_magic_numbers[from_square]) >> offset_RBits[from_square];
        possible_moves |= rook_attack_lookup_table[from_square][index];
        possible_moves &= not_active_pieces;    
        generate_moves_from_bitboard(ROOK, from_square, possible_moves, false);
        rook_bitboard &= ~(1ULL << (from_square));
    }

    // ========================= BISHOPS =========================
    ULL bishop_bitboard = active_pieces->bishops;
    while (bishop_bitboard){
        from_square = __builtin_ctzll(bishop_bitboard);
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = (bishop_blockers * actual_bishop_magic_numbers[from_square]) >> offset_BBits[from_square];
        possible_moves = bishop_attack_lookup_table[from_square][index];
        possible_moves &= not_active_pieces;
        generate_moves_from_bitboard(BISHOP, from_square, possible_moves, false);
        bishop_bitboard &= ~(1ULL << (from_square));
    }

    // ========================== KNIGHTS =========================
    ULL knight_bitboard = active_pieces->knights;
    while (knight_bitboard) {
        from_square = __builtin_ctzll(knight_bitboard);
        possible_moves = knight_attack_lookup_table[from_square];
        possible_moves &= not_active_pieces;
        generate_moves_from_bitboard(KNIGHT, from_square, possible_moves, false);
        knight_bitboard &= ~(1ULL << from_square);
    }


    // ========================== PAWNS =========================
    /** slightly complicated:
     * generate_moves_from_bitboard has special flags:
     * 1 = promotion, and numbers greater than 7 = en passant
    */
    ULL pawn_bitboard = active_pieces->pawns;
    ULL possible_promotions, capture_squares;
    ULL en_passant_square = 1ULL << position->en_passant;
    while (pawn_bitboard) {
        from_square = __builtin_ctzll(pawn_bitboard);
        uint8_t rank = from_square / 8;
        // possible pawn captures from lookup table:
        ULL attack_squares = pawn_attack_lookup_table[WHITE_TO_MOVE][from_square];
        possible_moves = attack_squares & (OPPONENT_PIECES->all_pieces);
        // square number for single pushes - used for avoiding repeating calculations
        uint8_t single_push_square = from_square + direction * 8;
        ULL single_push = 1ULL << (single_push_square);
    
        // single pushes (normal)
        if (single_push & ~all_pieces) {
            possible_moves |= single_push;
            // if single moves then possibly double moves:
            ULL double_push_square = 1ULL << (from_square + direction * 16);
            if ((rank == start_rank) && (double_push_square & ~all_pieces)) {
                // if double moves then set special flag to possible en passant square, and generate move
                generate_moves_from_bitboard(PAWN, from_square, double_push_square, single_push_square);
            }
        }
        // generate other possible moves if not promoting
        if (rank != seventh_rank) {
            generate_moves_from_bitboard(PAWN, from_square, possible_moves, 0);
        } else {
            // and if promoting:
            generate_moves_from_bitboard(PAWN, from_square, possible_moves, 1);
        }

        if (en_passant_square & attack_squares) {
            uint8_t en_passent_pawn_square = position->en_passant - direction * 8;
            generate_moves_from_bitboard(PAWN, from_square, en_passant_square, 100 + en_passent_pawn_square);
        }
        pawn_bitboard &= ~(1ULL << from_square);
    }


    // ========================== KINGS =========================
    ULL king_bitboard = active_pieces->kings;
    ULL threatened_squares = 0;

    ULL opponent_diagonals_bitboard = opponent_pieces->queens | opponent_pieces->bishops;
    while (opponent_diagonals_bitboard) {
        from_square = __builtin_ctzll(opponent_diagonals_bitboard);
        bishop_blockers = bishop_blocker_masks[from_square] & all_pieces;
        index = bishop_blockers * actual_bishop_magic_numbers[from_square] >> offset_BBits[from_square];
        threatened_squares |= bishop_attack_lookup_table[from_square][index];
        opponent_diagonals_bitboard &= ~(1ULL << from_square);
    }
    ULL opponent_straights_bitboard = opponent_pieces->queens | opponent_pieces->rooks;
    while (opponent_straights_bitboard) {
        from_square = __builtin_ctzll(opponent_straights_bitboard);
        rook_blockers = rook_blocker_masks[from_square] & all_pieces;
        index = rook_blockers * actual_rook_magic_numbers[from_square] >> offset_RBits[from_square];
        threatened_squares |= rook_attack_lookup_table[from_square][index];
        opponent_straights_bitboard &= ~(1ULL << from_square);
    }
    ULL opponent_knights = opponent_pieces->knights;
    while (opponent_knights) {
        from_square = __builtin_ctzll(opponent_knights);
        threatened_squares |= knight_attack_lookup_table[from_square];
        opponent_knights &= ~(1ULL << from_square);
    }
    ULL opponent_pawns = opponent_pieces->pawns;
    while (opponent_pawns) {
        from_square = __builtin_ctzll(opponent_pawns);
        threatened_squares |= pawn_attack_lookup_table[!WHITE_TO_MOVE][from_square];
        opponent_pawns &= ~(1ULL << from_square);
    }
    from_square = __builtin_ctzll(opponent_pieces->kings);
    threatened_squares |= king_attack_lookup_table[from_square];
    // print_bitboard(threatened_squares);

    from_square = __builtin_ctzll(king_bitboard);
    possible_moves = king_attack_lookup_table[from_square] & not_active_pieces & ~threatened_squares;
    generate_moves_from_bitboard(KING, from_square, possible_moves, 0);

    ULL must_be_empty = (threatened_squares | all_pieces) & castling_blocker_masks[WHITE_TO_MOVE][CASTLING_KINGSIDE];
    if (!must_be_empty && active_pieces->castle_kingside) {
        generate_moves_from_bitboard(KING, from_square, 1ULL << (from_square + 2), CASTLING_KINGSIDE);
    }
    must_be_empty = (threatened_squares | all_pieces) & castling_blocker_masks[WHITE_TO_MOVE][CASTLING_QUEENSIDE];
    if (!must_be_empty && active_pieces->castle_queenside) {
        generate_moves_from_bitboard(KING, from_square, 1ULL << (from_square - 2), CASTLING_QUEENSIDE);
    }
}




void move_maker(Position_t* const old_position, Position_t* new_position, Move_t* move)
{
    memcpy(new_position, old_position, sizeof(Position_t));
    ULL bitboard_move = 1ULL << move->from_square | 1ULL << move->to_square;
    ULL bitboard_from_square = 1ULL << move->from_square;
    ULL bitboard_to_square = 1ULL << move->to_square;

    PiecesOneColour_t* active_pieces;
    PiecesOneColour_t* opponent_pieces;
    if (move->is_white) {
        active_pieces = &new_position->white_pieces;
        opponent_pieces = &new_position->black_pieces;
    } else {
        active_pieces = &new_position->black_pieces;
        opponent_pieces = &new_position->white_pieces;
    }

    new_position->all_pieces ^= bitboard_move;

    switch (move->captured)
    {
        case NONE:
            break;
        case PAWN:
            opponent_pieces->pawns &= ~bitboard_to_square;
            opponent_pieces->all_pieces &= ~bitboard_to_square;
            break;
        case KNIGHT:
            opponent_pieces->knights &= ~bitboard_to_square;
            opponent_pieces->all_pieces &= ~bitboard_to_square;
            break;
        case BISHOP:
            opponent_pieces->bishops &= ~bitboard_to_square;
            opponent_pieces->all_pieces &= ~bitboard_to_square;
            break;
        case ROOK:
            opponent_pieces->rooks &= ~bitboard_to_square;
            opponent_pieces->all_pieces &= ~bitboard_to_square;
            break;
        case QUEEN:
            opponent_pieces->queens &= ~bitboard_to_square;
            opponent_pieces->all_pieces &= ~bitboard_to_square;
            break;
    }


    switch (move->special_flags)
    {
        ULL rook_move;

        case NORMAL_MOVE:
            switch (move->moved)
                {
                    case PAWN:
                        active_pieces->pawns ^= bitboard_move;
                        break;
                    case KNIGHT:
                        active_pieces->knights ^= bitboard_move;
                        break;
                    case BISHOP:
                        active_pieces->bishops ^= bitboard_move;
                        break;
                    case ROOK:
                        active_pieces->rooks ^= bitboard_move;
                        break;
                    case QUEEN:
                        active_pieces->queens ^= bitboard_move;
                        break;
                    case KING:
                        active_pieces->kings ^= bitboard_move;
                        break;
                }
                active_pieces->all_pieces ^= bitboard_move;
            break;

        case PROMOTION:
            active_pieces->pawns &= ~bitboard_from_square;
            switch (move->moved)
            {
                case KNIGHT:
                    active_pieces->knights |= bitboard_to_square;
                    break;
                case BISHOP:
                    active_pieces->bishops |= bitboard_to_square;
                    break;
                case ROOK:
                    active_pieces->rooks |= bitboard_to_square;
                    break;
                case QUEEN:
                    active_pieces->queens |= bitboard_to_square;
                    break;
            }
            break;
        
        case CASTLING_KINGSIDE:
            rook_move = 1ULL << (move->to_square - 1) | 1ULL << (move->to_square + 1);
            active_pieces->kings ^= bitboard_move;
            active_pieces->rooks ^= rook_move;
            active_pieces->all_pieces ^= (bitboard_move | rook_move);
            break;
        
        case CASTLING_QUEENSIDE:
            rook_move = 1ULL << (move->to_square + 1) | 1ULL << (move->to_square - 2);
            active_pieces->kings ^= bitboard_move;
            active_pieces->rooks ^= rook_move;
            active_pieces->all_pieces ^= (bitboard_move | rook_move);
            break;

        default:
            if (move->special_flags > 100) {
                ULL capture_square = ~(1ULL << (move->special_flags - 100));
                active_pieces->pawns ^= bitboard_move;
                opponent_pieces->pawns &= capture_square;
                opponent_pieces->all_pieces &= capture_square;
                new_position->all_pieces &= capture_square;
            } else {
                active_pieces->pawns ^= bitboard_move;
            }
            active_pieces->all_pieces ^= bitboard_move;
    }
}