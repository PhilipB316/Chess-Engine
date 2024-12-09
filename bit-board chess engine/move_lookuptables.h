/**
 * @file move_lookuptables.h
 * @brief Implementation of bitboards lookuptables for piece attacks.
 * @author Philip Brand
 * @date 2024-11-29
 * 
 * The lookuptables for all pieces are set as extern, so once generated using
 * the generate_lookup_tables(), the arrays are available to all other files.
 * 
 * TODO: Possibly implement magic numbers for a lookup table for two knights:
 * - instead of determining the possible moves for each knight individually, lookup
 * both at the same time using a magic index.
 */

#ifndef MAGIC_NUMBERS_H
#define MAGIC_NUMBERS_H

#include <stdint.h>

#define ULL unsigned long long
#define KNOWN_MAGIC_NUMBERS 1

// files and ranks used to prevent wraparound
#define FILE_A 0x0101010101010101
#define FILE_B 0x0202020202020202
#define FILE_G 0x4040404040404040
#define FILE_H 0x8080808080808080

#define RANK_1 0x00000000000000FF
#define RANK_2 0x000000000000FF00
#define RANK_7 0x00FF000000000000
#define RANK_8 0xFF00000000000000

extern ULL pawn_attack_lookup_table[2][64];
extern ULL knight_attack_lookup_table[64];
extern ULL king_attack_lookup_table[64];
extern ULL rook_attack_lookup_table[64][4096];
extern ULL bishop_attack_lookup_table[64][4096];
extern ULL magic_knight_attack_lookup_table[4096];

extern ULL rook_blocker_masks[64];
extern ULL bishop_blocker_masks[64];

// (64 - bits) numbers of bits rook blockers can occupy 
static uint8_t offset_RBits[64] = 
{
    52, 53, 53, 53, 53, 53, 53, 52,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    52, 53, 53, 53, 53, 53, 53, 52
};

static uint8_t offset_BBits[64] = {
    58, 59, 59, 59, 59, 59, 59, 58,
    59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59,
    58, 59, 59, 59, 59, 59, 59, 58
};

// prefound magic numbers for rooks
static ULL actual_rook_magic_numbers[64] = 
{
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
4684308548237328530ULL
};

// prefound magic numbers for bishops
static ULL actual_bishop_magic_numbers[64] = 
{
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
722917911187031552ULL
};


/**
 * @brief Generates the lookup tables for rook attacks using magic_numbers module
 * @param rook_attack_lookup_table the table to be populated
 */
void rook_attack_generator(void);


/**
 * @brief Generates the lookup tables for bishop attacks using magic_numbers module
 * @param bishop_attack_lookup_table the table to be populated
 */
void bishop_attack_generator(void);


/**
 * @brief Generates a lookup table for pawn attacks.
 * 
 * This function populates a lookup table that provides precomputed attack
 * bitboards for pawns. The table is indexed by pawn color (0 for white, 1 for black)
 * and the square index (0-63) representing the position of the pawn on the board.
 
 */
void pawn_attack_generator(void);


/**
 * @brief Generates a lookup table for knight attacks.
 * 
 * This function populates a lookup table that provides precomputed attack
 * bitboards for knights. The table is indexed by the square index (0-63) representing
 * the position of the knight on the board.
 * 
 * @param knight_attack_lookup_table An array to be filled with bitboards indicating
 *        the attack positions for knights.
 */
void knight_attack_generator(void);


/**
 * @brief Generates lookup tables for king attacks.
 * 
 * This function populates a lookup table that provides precomputed attack
 * bitboards for kings. The table is indexed by the square index (0-63) representing
 * the position of the king on the board.
 * 
 * @param king_attack_lookup_table An array to be filled with bitboards indicating
 *       the attack positions for kings.
 */
void king_attack_generator(void);


/**
 * @brief Generates lookup tables for piece attacks.
 *
 * This function initializes the provided lookup tables with precomputed
 * attack patterns - the lookup tables are used to quickly determine 
 * the possible moves for these pieces from any given position.
 *
 * @param lookup_tables The lookup tables to be filled with precomputed
 *  attack patterns.
 **/
void generate_lookup_tables(void);




#endif