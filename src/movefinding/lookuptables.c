/**
 * IMPLEMENTATION OF MAGIC NUMBERS:
 *
 * The blocker masks are a mask that is applied to all pieces on the board
 * to determine which pieces are blocking the rook or bishop from moving.
 * These pieces are called blockers.
 * Implementation of the masks is as: relevant_blockers = all_pieces & mask;
 * The magic numbers create a hash which relates each possible blocker permutation
 * to the possible moves for a piece with that given blocker configuration.
 * The magic numbers are generate by trial and error
 *
 * Generate lookup tables using masking and bit-shifting to determine the possible moves
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "lookuptables.h"
#include "board.h"

ULL rook_blocker_masks[64];
ULL bishop_blocker_masks[64];
ULL castling_blocker_masks[2][3];

ULL pawn_attack_lookup_table[2][64];
ULL knight_attack_lookup_table[64];
ULL king_attack_lookup_table[64];
ULL rook_attack_lookup_table[64][4096];
ULL bishop_attack_lookup_table[64][4096];
ULL magic_knight_attack_lookup_table[4096];

ULL rook_castling_array[2][2];
ULL king_castling_array[2][2];
ULL original_rook_locations[2][2];

ULL actual_bishop_magic_numbers[64] = {
    306249795545277056ULL,
    1162212637740662848ULL,
    289360676286103624ULL,
    5243390635171676304ULL,
    2882603104200097792ULL,
    4683893164334123008ULL,
    108369034312966208ULL,
    10448632889718345216ULL,
    1315196812053775361ULL,
    2305865171916357668ULL,
    17609404534784ULL,
    72625234104025600ULL,
    9297699607545978880ULL,
    12141846991017607238ULL,
    4613938402373935104ULL,
    27171286031832064ULL,
    73254000197371920ULL,
    2814889423077888ULL,
    4515730770891008ULL,
    1134704608805121ULL,
    19140850330107904ULL,
    35330409635840ULL,
    5766296684326815752ULL,
    35185530243074ULL,
    37735239604241921ULL,
    12106247587906006018ULL,
    36100342987360512ULL,
    563639429922976ULL,
    11097152056330305536ULL,
    11259548875620480ULL,
    148902530494497815ULL,
    288512132585228294ULL,
    36680056869494790ULL,
    5642702541828096ULL,
    39754220437632ULL,
    54641884442132992ULL,
    2607584742593274368ULL,
    1157442701017251904ULL,
    9313731006232135746ULL,
    143076114908165ULL,
    9226754814304585732ULL,
    5333392267665213472ULL,
    9576754884907520ULL,
    243352993028900865ULL,
    612947504647898112ULL,
    27056971119068192ULL,
    1298237361628779520ULL,
    2278464086360128ULL,
    28151908957946880ULL,
    864973187564767744ULL,
    578713668826696704ULL,
    288309750402252864ULL,
    36029140650033236ULL,
    4611699299675931792ULL,
    4591564869607496ULL,
    1157442705014538240ULL,
    109989843701824ULL,
    11003723073600ULL,
    37155258123751426ULL,
    72075753168315402ULL,
    297307978783131144ULL,
    10178173658079437056ULL,
    5332297169252787209ULL,
    722917911187031552ULL};

ULL actual_rook_magic_numbers_apparently_bad[64] = {
    756607761056301088ULL,
    4917965982829391872ULL,
    9259436018514335872ULL,
    252227969560612864ULL,
    1873504042192277572ULL,
    4683744712145502216ULL,
    10736588108788171008ULL,
    72060078754236160ULL,
    140880296050688ULL,
    36239972015016000ULL,
    288793611754082336ULL,
    10980338876383625296ULL,
    281562084086016ULL,
    7219973898715005952ULL,
    288511855423520780ULL,
    703743277449344ULL,
    10430337286754283586ULL,
    4616189892934762561ULL,
    845524712292416ULL,
    738733825827278848ULL,
    81910317868647472ULL,
    10133648984637568ULL,
    27025995828578306ULL,
    24807181388087505ULL,
    4611826895502458880ULL,
    290517918687560456ULL,
    436853014294175744ULL,
    3891391591680573472ULL,
    8798240768384ULL,
    54608346652868736ULL,
    36037885170811138ULL,
    4621327094725559297ULL,
    18020034588704898ULL,
    794920529348804608ULL,
    38984971611803648ULL,
    4611765185420464128ULL,
    653444742557733889ULL,
    9379309380144470164ULL,
    4616304006254301322ULL,
    5089704017996ULL,
    2305983773545627684ULL,
    1478940450466431008ULL,
    282162185437249ULL,
    36310615627333664ULL,
    145241122350858244ULL,
    563259460157452ULL,
    77124178800672772ULL,
    2533275872591874ULL,
    306325047602086400ULL,
    9876464420289249344ULL,
    7206920489413936256ULL,
    5440992801384890624ULL,
    290490978518433920ULL,
    4400196747392ULL,
    9377629124748796928ULL,
    4611734676179550720ULL,
    72093878194930306ULL,
    1299358895885811745ULL,
    3603162595299696641ULL,
    1424967491126529ULL,
    72339077738795013ULL,
    563518231807746ULL,
    4505833060814860ULL,
    4684308548237328530ULL};

ULL actual_rook_magic_numbers[64] = {
    2341871877105926144ULL,
    450361887150841856ULL,
    72075568480264192ULL,
    180170375522418932ULL,
    72083982585757714ULL,
    468387555402973312ULL,
    36030996050624768ULL,
    72062542930772258ULL,
    4630122631553613856ULL,
    9368050320917168384ULL,
    281612486971652ULL,
    1829656076552193ULL,
    2306406036476862472ULL,
    140746086683648ULL,
    4900197873853005828ULL,
    148900263763247872ULL,
    9324408401838080ULL,
    2389309686731251712ULL,
    171137335864729728ULL,
    5228679717409785856ULL,
    54184482906866688ULL,
    37155246715207684ULL,
    9295570919311284480ULL,
    3476851480374349900ULL,
    19210669308551296ULL,
    11529250231515349056ULL,
    9228448215754346624ULL,
    36037670423625792ULL,
    600388345822445584ULL,
    288793334696117256ULL,
    117111199678076419ULL,
    17910013895811ULL,
    2322169103122816ULL,
    2341977395864551428ULL,
    140874952478720ULL,
    2903733281813958656ULL,
    4701898765651412994ULL,
    18863223641997824ULL,
    54606163299279112ULL,
    18095215936012548ULL,
    5800654049715994626ULL,
    4503737347457024ULL,
    54327281846386752ULL,
    2251868533719168ULL,
    2305869397694120064ULL,
    9511747548574941312ULL,
    7494271263513313316ULL,
    288847340696109060ULL,
    576497141413855488ULL,
    9011737962480704ULL,
    10212337015324928ULL,
    2323875002596393216ULL,
    36033205803417728ULL,
    72761290086678656ULL,
    585555955591676928ULL,
    144683637806465536ULL,
    281750392668290ULL,
    90107451865728259ULL,
    2042893008117890ULL,
    247392408702977ULL,
    78250181032153094ULL,
    6053119382953920515ULL,
    576637327117386244ULL,
    578713668821459858ULL};


uint8_t offset_RBits[64] = {
    52, 53, 53, 53, 53, 53, 53, 52,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    52, 53, 53, 53, 53, 53, 53, 52};

uint8_t offset_BBits[64] = {
    58, 59, 59, 59, 59, 59, 59, 58,
    59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59,
    58, 59, 59, 59, 59, 59, 59, 58};

// arrays for storing magic numbers for rooks and bishops if looking for them
static ULL array_for_rook_magic_numbers[64];
static ULL array_for_bishop_random_numbers[64];

void generate_lookup_tables(void)
{
    pawn_attack_generator();
    knight_attack_generator();
    king_attack_generator();
    rook_attack_generator();
    bishop_attack_generator();
}

uint16_t random(void)
{
    // This function should return a random number.
    // For simplicity, we can use rand() from stdlib.h.
    // In a real chess engine, you might want to use a better random number generator.
    return rand() % 65536; // Return a random number between 0 and 65535
}

/**
 * @brief Generates a random unsigned long long integer.
 *
 * This function utilizes the C standard library's random number generation
 * facilities to produce a random unsigned long long integer. It ensures
 * that the generated number is within the range of an unsigned long long.
 *
 * @return A randomly generated unsigned long long integer.
 */
ULL random_ULL(void)
{
    ULL u1, u2, u3, u4;
    // srand(time(NULL));
    u1 = (ULL)(random()) & 0xFFFF;
    u2 = (ULL)(random()) & 0xFFFF;
    u3 = (ULL)(random()) & 0xFFFF;
    u4 = (ULL)(random()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

/**
 * @brief Generates a random unsigned long long integer with a few bits set as 1.
 *
 * This is necessary as the magic numbers tend to have this number of bits set as 1.
 *
 * @return A random unsigned long long integer with a few bits set.
 */
ULL random_ULL_fewbits(void)
{
    return random_ULL() & random_ULL() & random_ULL();
}

/**
 * @brief Generates the blocker masks for piece for all squares on the board.
 * Iterates through each direction until it reaches the edge of the board - although:
 * the actual edge of the board is not necessary as a piece on the very edge has the
 * same effect as the edge of the board.
 */
void generate_rook_blocker_masks(void)
{
    for (int square = 0; square < 64; square++)
    {
        ULL mask = 0;
        int square_rank = square / 8;
        int square_file = square % 8;

        // north direction
        for (int i = square_rank + 1; i < 7; i++) { mask |= (1ULL << (square_file + i * 8)); }
        // south direction
        for (int i = square_rank - 1; i > 0; i--) { mask |= (1ULL << (square_file + i * 8)); }
        // east direction
        for (int i = square_file + 1; i < 7; i++) { mask |= (1ULL << (i + square_rank * 8)); }
        // west direction
        for (int i = square_file - 1; i > 0; i--) { mask |= (1ULL << (i + square_rank * 8)); }

        rook_blocker_masks[square] = mask;
    }
}

void generate_bishop_blocker_masks(void)
{
    for (int square = 0; square < 64; square++)
    {
        ULL mask = 0ULL;
        int square_rank = square / 8;
        int square_file = square % 8;

        // north-east direction
        for (int r = square_rank + 1, f = square_file + 1; r < 7 && f < 7; r++, f++)
        {
            mask |= (1ULL << (f + r * 8));
        }
        // north-west direction
        for (int r = square_rank + 1, f = square_file - 1; r < 7 && f > 0; r++, f--)
        {
            mask |= (1ULL << (f + r * 8));
        }
        // south-east direction
        for (int r = square_rank - 1, f = square_file + 1; r > 0 && f < 7; r--, f++)
        {
            mask |= (1ULL << (f + r * 8));
        }
        // south-west direction
        for (int r = square_rank - 1, f = square_file - 1; r > 0 && f > 0; r--, f--)
        {
            mask |= (1ULL << (f + r * 8));
        }

        bishop_blocker_masks[square] = mask;
    }
}

/**
 * @brief Determines the possible moves for a piece given a blocker configuration.
 * Iterates through each direction until it reaches a blocker or the edge of the board.
 *
 * @param sq The square index of the piece.
 * @param blocker The blocker configuration.
 * @return The possible moves for the piece.
 */

ULL determine_possible_rook_moves(uint8_t sq, ULL blocker)
{
    ULL mask = 0ULL;
    uint8_t square_rank = sq / 8;
    uint8_t square_file = sq % 8;
    int rank, file;
    uint8_t possible_square;

    // north direction
    for (rank = square_rank + 1; rank <= 7; rank++)
    {
        possible_square = square_file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }
    // south direction
    for (rank = square_rank - 1; rank >= 0; rank--)
    {
        possible_square = square_file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }
    // east direction
    for (file = square_file + 1; file <= 7; file++)
    {
        possible_square = file + square_rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }
    // west direction
    for (file = square_file - 1; file >= 0; file--)
    {
        possible_square = file + square_rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }
    return mask;
}

ULL determine_possible_bishop_moves(uint8_t sq, ULL blocker)
{
    ULL mask = 0ULL;
    uint8_t square_rank = sq / 8;
    uint8_t square_file = sq % 8;
    int rank, file;
    uint8_t possible_square;

    // north-east direction
    for (rank = square_rank + 1, file = square_file + 1; rank <= 7 && file <= 7; rank++, file++)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }
    // north-west direction
    for (rank = square_rank + 1, file = square_file - 1; rank <= 7 && file >= 0; rank++, file--)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }
    // south-east direction
    for (rank = square_rank - 1, file = square_file + 1; rank >= 0 && file <= 7; rank--, file++)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }
    // south-west direction
    for (rank = square_rank - 1, file = square_file - 1; rank >= 0 && file >= 0; rank--, file--)
    {
        possible_square = file + rank * 8;
        mask |= (1ULL << possible_square);
        if (blocker & (1ULL << possible_square))
            break;
    }

    return mask;
}

ULL determine_possible_knight_moves(ULL bitboard)
{
    ULL possible_moves = 0;
    for (int i = 0; i < 64; i++)
    {
        if (bitboard & (1ULL << i))
        {
            possible_moves |= knight_attack_lookup_table[i];
        }
    }
    return possible_moves;
}

/**
 * @brief Generates all possible blockers and their corresponding magic numbers.
 *
 * FOR EVERY SQUARE ON THE BOARD (although the function is called individually for each square):
 *
 * A random magic number is generated.
 *
 * The number of permutations of blockers is found by counting the number of bits
 * in the blocker mask relating to that square.
 * For each permutation of blockers, the possible moves for the rook with that blocker configuration
 * are determined using the determine_possible_rook_moves function.
 *
 * The hash index for each permutation is then found,
 * using the formula: index = (blocker * magic_number) >> (64 - bits)
 *
 * The value at the index in the lookup table is then checked, and if there is no collision,
 * the possible moves are stored in the lookup table at that index.
 * No collision means that the value at the index is either 0 or the same as the possible moves.
 *
 * If there is a collision, the function returns false and the entire process is repeated for that square.
 * Once the function returns true, the magic number is stored in the magic number array,
 * and a new square is passed into the function.
 *
 *
 * The function has three parts:
 * - selection of appropriate tables depending on the piece type
 * - generation of appropriate blocker permutation
 * - testing and assignment of magic number
 *
 * Typical usage is implemented as in the following example:
 *
    for (int j = 0; j < 64; j++)
    {
        printf("Square %d\n", j);
        while (!generate_possible_blockers_and_magic_numbers(j, false))
        {
            // clear the lookup table at that square if there is a collision //
            for (int i = 0; i < 4096; i++)
            {
                bishop_attack_lookup_table[j][i] = 0;
            }
        }
    }
 *
 * @param square The square index of the piece.
 * @param rook true if the piece is a rook, false if it is a bishop
 * @return true if the magic number was generated successfully, false otherwise.
 */
bool generate_possible_blockers_and_magic_numbers(uint8_t square, bool rook)
{
    ULL *magic_numbers_array;
    ULL *blocker_masks;
    if (rook)
    {
        magic_numbers_array = actual_rook_magic_numbers;
        blocker_masks = rook_blocker_masks;
    }
    else
    {
        magic_numbers_array = actual_bishop_magic_numbers;
        blocker_masks = bishop_blocker_masks;
    }

    ULL mask = blocker_masks[square];
    int bits = __builtin_popcountll(mask);
    int permutations = 1 << bits;
    ULL magic_number;

    if (KNOWN_MAGIC_NUMBERS)
    {
        magic_number = magic_numbers_array[square];
    }
    else
    {
        magic_number = random_ULL_fewbits();
    }

    // all the permutations are simply numbers from 0 to 2^bits
    // this code sets the blocker mask bits to that permutation value
    for (int i = 0; i < permutations; i++)
    {
        ULL blocker = 0;
        int bit_index = 0;
        // iterates through all the squares on the board
        for (int j = 0; j < 64; j++)
        {
            // if the square on the board is a relevant blocker
            if (mask & (1ULL << j))
            {
                // then set the bit if the bit_index-th bit of the permutation value is set
                if (i & (1 << bit_index))
                {
                    blocker |= (1ULL << j);
                }
                bit_index++;
            }
        }

        if (rook)
        {
            // determine possible moves for that permutation
            ULL possible_moves = determine_possible_rook_moves(square, blocker);
            ULL index = (blocker * magic_number) >> (64 - bits);
            ULL current = rook_attack_lookup_table[square][index];

            if (KNOWN_MAGIC_NUMBERS)
            {
                rook_attack_lookup_table[square][index] = possible_moves;
                array_for_rook_magic_numbers[square] = magic_number;
            }
            else
            {
                // if there is no collision, store the possible moves in the lookup table
                if (current == possible_moves) { continue; }
                else if (!current) { rook_attack_lookup_table[square][index] = possible_moves; }
                else { return 0; } }
        }
        else
        {

            ULL possible_moves = determine_possible_bishop_moves(square, blocker);
            ULL index = (blocker * magic_number) >> (64 - bits);
            ULL current = bishop_attack_lookup_table[square][index];

            if (KNOWN_MAGIC_NUMBERS)
            {
                bishop_attack_lookup_table[square][index] = possible_moves;
                array_for_bishop_random_numbers[square] = magic_number;
            }
            else
            {
                if (current == possible_moves) { continue; }
                else if (!current) { bishop_attack_lookup_table[square][index] = possible_moves; }
                else { return 0; }
            }
        }
    }

    if (!KNOWN_MAGIC_NUMBERS)
    {
        if (rook) { array_for_rook_magic_numbers[square] = magic_number; }
        else { array_for_bishop_random_numbers[square] = magic_number; }
    }
    return 1;
}

/**not successfully generating magic numbers :-( */
bool generate_magic_numbers_for_knights(void)
{
    ULL possible_permutation;
    ULL magic_number = random_ULL_fewbits();
    printf("Magic number: %llu\n", magic_number);
    for (uint8_t i = 0; i < 64; i++)
    {
        // printf("Square %d\n", i);
        for (uint8_t j = 0; j < 64; j++)
        {
            possible_permutation = 1ULL << i;
            possible_permutation |= 1ULL << j;
            // print_bitboard(possible_permutation);
            ULL possible_moves = determine_possible_knight_moves(possible_permutation);
            // print_bitboard(possible_moves);
            // printf("i, j: %d, %d\n", i, j);
            ULL index = (possible_moves * magic_number) >> 52;
            // printf("Permutation: %llu\n", possible_permutation);
            // print_bitboard(possible_permutation);
            // printf("Moves: %llu\n", possible_moves);
            // print_bitboard(possible_moves);
            printf("Index: %llu\n", index);

            if (magic_knight_attack_lookup_table[index] == 0 || magic_knight_attack_lookup_table[index] == possible_moves)
            {
                magic_knight_attack_lookup_table[index] = possible_moves;
            }
            else
            {
                for (int i = 0; i < 4096; i++) { magic_knight_attack_lookup_table[i] = 0; }
                return false;
            }
        }
    }
    return true;
}

void rook_attack_generator(void)
{
    generate_rook_blocker_masks();
    for (int square = 0; square < 64; square++)
    {
        while (!generate_possible_blockers_and_magic_numbers(square, true))
        {
            for (int i = 0; i < 4096; i++)
            {
                rook_attack_lookup_table[square][i] = 0;
            }
        }
    }
}

void bishop_attack_generator(void)
{
    generate_bishop_blocker_masks();
    for (int square = 0; square < 64; square++)
    {
        while (!generate_possible_blockers_and_magic_numbers(square, false))
        {
            for (int i = 0; i < 4096; i++)
            {
                bishop_attack_lookup_table[square][i] = 0;
            }
        }
    }
}

void pawn_attack_generator(void)
{
    // white attacks (heading north, starting from square 55)
    for (int i = 55; i > 8; i--)
    {
        ULL pawn = 1ULL << i;
        ULL pawn_attack = 0;
        // masks
        ULL left_attack = (pawn & ~FILE_A) >> 9;
        ULL right_attack = (pawn & ~FILE_H) >> 7;
        pawn_attack |= left_attack | right_attack;
        // 1 index for white, 0 index for black
        pawn_attack_lookup_table[1][i] = pawn_attack;
    }
    // black attacks (heading south, starting from square 7)
    for (int i = 7; i < 56; i++)
    {
        ULL pawn = 1ULL << i;
        ULL pawn_attack = 0;
        // masks
        ULL left_attack = (pawn & ~FILE_A) << 7;
        ULL right_attack = (pawn & ~FILE_H) << 9;
        pawn_attack |= left_attack | right_attack;
        pawn_attack_lookup_table[0][i] = pawn_attack;
    }
}

void knight_attack_generator(void)
{
    for (int i = 0; i < 64; i++)
    {
        ULL knight = 1ULL << i;
        ULL knight_attack = 0;

        // masks to prevent wraparound
        ULL hard_left_up = (knight & ~FILE_A & ~FILE_B & ~RANK_1) >> 10;
        ULL soft_left_up = (knight & ~FILE_A & ~RANK_1 & ~RANK_2) >> 17;
        ULL hard_right_up = (knight & ~FILE_H & ~FILE_G & ~RANK_1) >> 6;
        ULL soft_right_up = (knight & ~FILE_H & ~RANK_1 & ~RANK_2) >> 15;

        ULL hard_left_down = (knight & ~FILE_A & ~FILE_B & ~RANK_8) << 6;
        ULL soft_left_down = (knight & ~FILE_A & ~RANK_7 & ~RANK_8) << 15;
        ULL hard_right_down = (knight & ~FILE_H & ~FILE_G & ~RANK_8) << 10;
        ULL soft_right_down = (knight & ~FILE_H & ~RANK_7 & ~RANK_8) << 17;

        knight_attack |= hard_left_up | soft_left_up | hard_right_up | soft_right_up | hard_left_down | soft_left_down | hard_right_down | soft_right_down;

        knight_attack_lookup_table[i] = knight_attack;
        // printf("knight_attack_lookup_table[%d]: %llu\n", i, knight_attack_lookup_table[i]);
    }
}

void king_attack_generator(void)
{
    for (int i = 0; i < 64; i++)
    {
        ULL king = 1ULL << i;
        ULL king_attack = 0;

        // masks to prevent wraparound
        ULL left = (king & ~FILE_A) >> 1;
        ULL right = (king & ~FILE_H) << 1;
        ULL up = (king & ~RANK_1) >> 8;
        ULL down = (king & ~RANK_8) << 8;

        ULL left_up = (king & ~FILE_A & ~RANK_1) >> 9;
        ULL right_up = (king & ~FILE_H & ~RANK_1) >> 7;
        ULL left_down = (king & ~FILE_A & ~RANK_8) << 7;
        ULL right_down = (king & ~FILE_H & ~RANK_8) << 9;

        king_attack |= left | right | up | down | left_up | right_up | left_down | right_down;

        king_attack_lookup_table[i] = king_attack;

        // move tables for rooks and kings castling
        rook_castling_array[0][0] = 1ULL << 7 | 1ULL << 5;
        rook_castling_array[0][1] = 1ULL << 0 | 1ULL << 2;
        rook_castling_array[1][0] = 1ULL << 63 | 1ULL << 61;
        rook_castling_array[1][1] = 1ULL << 56 | 1ULL << 58;

        king_castling_array[0][0] = 1ULL << 6;
        king_castling_array[0][1] = 1ULL << 1;
        king_castling_array[1][0] = 1ULL << 62;
        king_castling_array[1][1] = 1ULL << 57;

        original_rook_locations[!WHITE_INDEX][QUEENSIDE] = 1ULL << 56;
        original_rook_locations[!WHITE_INDEX][!QUEENSIDE] = 1ULL << 63;
        original_rook_locations[WHITE_INDEX][QUEENSIDE] = 1ULL << 0;
        original_rook_locations[WHITE_INDEX][!QUEENSIDE] = 1ULL << 7;

        // these masks include the king square as the king cannot be in check and castle at the same time
        castling_blocker_masks[!WHITE_INDEX][!QUEENSIDE] = 1ULL << 4 | 1ULL << 5 | 1ULL << 6;
        castling_blocker_masks[!WHITE_INDEX][QUEENSIDE] = 1ULL << 1 | 1ULL << 2 | 1ULL << 3 | 1ULL << 4;
        castling_blocker_masks[WHITE_INDEX][!QUEENSIDE] = 1ULL << 60 | 1ULL << 61 | 1ULL << 62;
        castling_blocker_masks[WHITE_INDEX][QUEENSIDE] = 1ULL << 57 | 1ULL << 58 | 1ULL << 59 | 1ULL << 60;

    }
}
