/**
 * @file uci.c
 * @brief UCI (Universal Chess Interface) protocol implementation.
 * @author Philip Brand
 * @date 2026-06-09
 */


#include "uci.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include "../movefinding/board.h"
#include "../movefinding/movefinder.h"
#include "../search/search.h"
#include "../search/hash_tables.h"


#define UCI_STARTPOS \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define UCI_LINE_BUF 4096
#define UCI_MOVE_BUF 6 
#define DEFAULT_DEPTH 20
#define DEFAULT_MS 5000LL
#define INFINITE_MS 1000000ULL
#define UNSET_SEARCH_TIME 87234ULL

static Position_t g_pos;    /* current game position, set by handle_position */
static Position_t g_result; /* best-move result, filled by handle_go         */

static int g_skill_level = 5;

static uint8_t sq_to_idx(char file, char rank)
{ return (uint8_t)(('8' - rank) * 8 + (file - 'a')); }


static void idx_to_sq(uint8_t idx, char *out)
{
    out[0] = (char)('a' + idx % 8);
    out[1] = (char)('8' - idx / 8);
}




/**
 * @brief Extract the next whitespace-delimited token from *p.
 *
 * Writes at most buf_len-1 characters into @p buf (NUL-terminated),
 * then advances *p past the token and any following spaces.
 *
 * @return Number of characters written (0 if already at end of string).
 */
static size_t next_token(const char **p, char *buf, size_t buf_len)
{
    while (**p == ' ') (*p)++;
    size_t i = 0;
    while (**p && **p != ' ' && i < buf_len - 1)
        buf[i++] = *(*p)++;
    buf[i] = '\0';
    while (**p == ' ') (*p)++;
    return i;
}


static MoveType_t move_type_from_uci(const Position_t *pos,
                                     uint8_t from, uint8_t to, char promo)
{
    const PiecesOneColour_t *active = &pos->pieces[pos->white_to_move];
    ULL from_bb = 1ULL << from;

    /* ── pawn ── */
    if (from_bb & active->pawns) {
        if (promo) {
            switch (tolower((unsigned char)promo)) {
                case 'q': return PROMOTE_QUEEN;
                case 'r': return PROMOTE_ROOK;
                case 'b': return PROMOTE_BISHOP;
                case 'n': return PROMOTE_KNIGHT;
                default:  return PROMOTE_QUEEN; /* malformed → queen */
            }
        }
        if (pos->en_passant_bitboard &&
            ((1ULL << to) == pos->en_passant_bitboard))
        { return EN_PASSANT_CAPTURE; }

        /* double push: rank distance == 2 */
        if (abs((int)(from / 8) - (int)(to / 8)) == 2)
            return DOUBLE_PUSH;

        return PAWN;
    }

    /* ── sliding / leaping pieces ── */
    if (from_bb & active->knights) return KNIGHT;
    if (from_bb & active->bishops) return BISHOP;
    if (from_bb & active->rooks)   return ROOK;
    if (from_bb & active->queens)  return QUEEN;

    /* ── king (includes castling) ── */
    if (from_bb & active->kings) {
        int df = (int)(to % 8) - (int)(from % 8);
        if (df ==  2) return CASTLE_KINGSIDE;
        if (df == -2) return CASTLE_QUEENSIDE;
        return KING;
    }

    return PAWN; /* should never reach here */
}




/**
 * @brief Parse and apply one UCI move string to *src, writing the result
 *        into *dst.
 *
 * Accepts 4-char normal moves ("e2e4") and 5-char promotion moves ("e7e8q").
 *
 * @return true on success, false if the move string is malformed or illegal.
 */
static bool apply_uci_move(Position_t *src, Position_t *dst, const char *mv)
{
    if (!mv || strlen(mv) < 4) return false;

    uint8_t from  = sq_to_idx(mv[0], mv[1]);
    uint8_t to    = sq_to_idx(mv[2], mv[3]);
    char    promo = mv[4] ? (char)tolower((unsigned char)mv[4]) : '\0';

    MoveType_t mtype = move_type_from_uci(src, from, to, promo);

    /* en_passant_bb param: pass the ep target square for ep captures,
       0 for everything else (matches the convention in make_notation_move). */
    ULL ep_bb = (mtype == EN_PASSANT_CAPTURE) ? src->en_passant_bitboard : 0ULL;

    return make_notation_move(src, dst, mtype,
                              /*to_bb*/   1ULL << to,
                              /*from_bb*/ 1ULL << from,
                              ep_bb);
}

/* ══════════════════════════════════════════════════════════════════
   Bestmove string construction
   ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Build the UCI bestmove string into @p out (min UCI_MOVE_BUF bytes).
 *
 * Uses from_sq / to_sq stored in *after by find_best_move, then detects
 * promotions by comparing the mover's piece bitboards before and after.
 * Only checks for promotion if the moving piece was actually a pawn, to
 * avoid false positives on normal queen/rook moves.
 */
static void make_bestmove_str(const Position_t *before,
                              const Position_t *after,
                              char *out)
{
    uint8_t from = after->from_sq;
    uint8_t to   = after->to_sq;

    idx_to_sq(from, out);      /* out[0], out[1] */
    idx_to_sq(to,   out + 2);  /* out[2], out[3] */
    out[4] = '\0';
    out[5] = '\0';

    /* Only a pawn can promote; guard against false matches on queen moves. */
    bool mover = before->white_to_move;
    if (!(before->pieces[mover].pawns & (1ULL << from))) return;

    ULL to_bb = 1ULL << to;
    const PiecesOneColour_t *bp = &before->pieces[mover];
    const PiecesOneColour_t *ap = &after->pieces[mover];

    if      ((ap->queens  & to_bb) && !(bp->queens  & to_bb)) out[4] = 'q';
    else if ((ap->rooks   & to_bb) && !(bp->rooks   & to_bb)) out[4] = 'r';
    else if ((ap->bishops & to_bb) && !(bp->bishops & to_bb)) out[4] = 'b';
    else if ((ap->knights & to_bb) && !(bp->knights & to_bb)) out[4] = 'n';
}

static void handle_uci(void)
{
    printf("id name TessMax %d.%d.%d\n",
           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    printf("id author Philip Brand\n");
    // printf("option name Hash type spin default 16 min 1 max 1024\n");
    printf("option name Skill Level type spin default 5 min 0 max 10\n");
    printf("uciok\n");
    fflush(stdout);
}

static void handle_isready(void)
{
    printf("readyok\n");
    fflush(stdout);
}

static void handle_setoption(const char *line)
{
    /* line format: "setoption name <name> value <value>" */
    const char *p = line;
    char name_buf[128] = {0};
    char value_buf[128] = {0};

    /* find "name" */
    const char *name_kw = strstr(p, "name");
    if (!name_kw) return;
    name_kw += 4;
    while (*name_kw == ' ') name_kw++;

    /* copy name up to " value" or end */
    size_t ni = 0;
    const char *iter = name_kw;
    while (*iter && strncmp(iter, " value", 6) != 0 && ni < sizeof(name_buf)-1)
        name_buf[ni++] = *iter++;
    name_buf[ni] = '\0';

    /* find "value" */
    const char *value_kw = strstr(p, "value");
    if (value_kw) {
        value_kw += 5;
        while (*value_kw == ' ') value_kw++;
        /* copy remainder as value (trim trailing spaces) */
        size_t vi = 0;
        while (*value_kw && vi < sizeof(value_buf)-1)
            value_buf[vi++] = *value_kw++;
        while (vi > 0 && value_buf[vi-1] == ' ') vi--;
        value_buf[vi] = '\0';
    }

    if (strcmp(name_buf, "Skill Level") == 0 && value_buf[0] != '\0') {
        int v = atoi(value_buf);
        if (v < 0) v = 0;
        if (v > 10) v = 10;
        g_skill_level = v;
    }
    /* TODO: parse "name Hash value <N>" to resize hash table, etc. */
}

static void handle_ucinewgame(void)
{
    fen_to_board(UCI_STARTPOS, &g_pos);
    insert_past_move_entry(&g_pos);
}

/**
 * @brief Handle: position [startpos | fen <fenstring>] [moves <m1> <m2> ...]
 *
 * Replays the full move list from scratch on every call. This is the
 * standard stateless approach: correct, simple, and safe.
 *
 * See file-level NOTE about repetition history accumulation.
 */
static void handle_position(const char *line)
{
    char fen[FEN_LENGTH];
    const char *p = line + 8; /* skip "position" */
    while (*p == ' ') p++;

    /* ── resolve base FEN ── */
    if (strncmp(p, "startpos", 8) == 0) {
        strncpy(fen, UCI_STARTPOS, FEN_LENGTH - 1);
        fen[FEN_LENGTH - 1] = '\0';
        p += 8;
    } else if (strncmp(p, "fen", 3) == 0) {
        p += 3;
        while (*p == ' ') p++;
        /* FEN ends at " moves" keyword or end of string. */
        const char *fen_end = strstr(p, " moves");
        if (!fen_end) fen_end = p + strlen(p);
        size_t n = (size_t)(fen_end - p);
        if (n >= FEN_LENGTH) n = FEN_LENGTH - 1;
        strncpy(fen, p, n);
        fen[n] = '\0';
        p = fen_end;
    } else {
        /* malformed — fall back to starting position */
        strncpy(fen, UCI_STARTPOS, FEN_LENGTH - 1);
        fen[FEN_LENGTH - 1] = '\0';
    }

    fen_to_board(fen, &g_pos);
    insert_past_move_entry(&g_pos);

    /* ── replay move list ── */
    const char *mv_kw = strstr(p, "moves");
    if (!mv_kw) return;
    p = mv_kw + 5; /* skip "moves" */

    char token[UCI_MOVE_BUF];
    Position_t next;

    while (next_token(&p, token, sizeof(token)) > 0) {
        if (!apply_uci_move(&g_pos, &next, token)) {
            fprintf(stderr, "UCI: illegal move in position list: %s\n", token);
            break; /* keep g_pos at the last valid state */
        }
        g_pos = next;
        insert_past_move_entry(&g_pos);
    }
}


void difficulty_to_search_params(int difficulty, int* time_ms, uint8_t* max_depth)
{
    const double base = 20.0;
    const double growth = 1.9;
    *time_ms = (int)(base * pow(growth, difficulty - 1));
    static const uint8_t depth_caps[11] = {0, 1, 2, 3, 4, 6, 9, 13, 17, 25, 60 };
    *max_depth = depth_caps[difficulty];
}


/**
 * @brief Handle: go [wtime <ms>] [btime <ms>] [winc <ms>] [binc <ms>]
 *                   [movetime <ms>] [depth <n>] [infinite]
 *
 * Runs find_best_move() then immediately outputs "bestmove <move>".
 * Does NOT update g_pos — the GUI will resend the full position command
 * (including the engine's reply) before the next go.
 */
static void handle_go(const char *line)
{
    long long search_ms    = DEFAULT_MS;
    uint8_t   search_depth = DEFAULT_DEPTH;

    long long wtime = 0, btime = 0, winc = 0, binc = 0;
    bool      has_clock = false;

    const char *p = line + 2; /* skip "go" */
    char token[32], val[32];
    search_ms = UNSET_SEARCH_TIME;

    while (next_token(&p, token, sizeof(token)) > 0) {

        if (strcmp(token, "infinite") == 0) {
            search_ms    = INFINITE_MS;
            search_depth = MAX_SEARCH_DEPTH;

        } else if (strcmp(token, "movetime") == 0) {
            next_token(&p, val, sizeof(val));
            search_ms = atoll(val);

        } else if (strcmp(token, "depth") == 0) {
            next_token(&p, val, sizeof(val));
            search_depth = (uint8_t)atoi(val);
            if (search_ms == UNSET_SEARCH_TIME) { search_ms = INFINITE_MS; }

        } else if (strcmp(token, "wtime") == 0) {
            next_token(&p, val, sizeof(val));
            wtime = atoll(val); has_clock = true;

        } else if (strcmp(token, "btime") == 0) {
            next_token(&p, val, sizeof(val));
            btime = atoll(val); has_clock = true;

        } else if (strcmp(token, "winc") == 0) {
            next_token(&p, val, sizeof(val));
            winc = atoll(val);

        } else if (strcmp(token, "binc") == 0) {
            next_token(&p, val, sizeof(val));
            binc = atoll(val);
        }
        /* movestogo, nodes, mate, searchmoves, ponder: silently ignored */
    }

    /* ── time management when clock times are provided ── */
    if (has_clock) {
        long long my_time = g_pos.white_to_move ? wtime : btime;
        long long my_inc  = g_pos.white_to_move ? winc  : binc;
        search_ms = my_time / 30 + my_inc;
        if (search_ms < 50LL)          search_ms = 50LL;
        if (search_ms > my_time / 2)   search_ms = my_time / 2;
    }

    int possible_search_time;
    difficulty_to_search_params(g_skill_level, &possible_search_time, &search_depth);
    search_ms = (possible_search_time > search_ms) ? search_ms : possible_search_time;
    // printf("search time: %d\n", search_ms);
    // printf("search level: %d\n", g_skill_level);
    // printf("search depth: %d\n", search_depth);

    /* ── search ── */
    find_best_move(&g_pos, &g_result, search_depth, search_ms);

    /* ── output ── */
    char move_str[UCI_MOVE_BUF] = {0};
    make_bestmove_str(&g_pos, &g_result, move_str);
    printf("bestmove %s\n", move_str);
    fflush(stdout);
}




/* ══════════════════════════════════════════════════════════════════
   Main UCI loop
   ══════════════════════════════════════════════════════════════════ */

void uci_loop(void)
{
    char line[UCI_LINE_BUF];

    /* Seed with the starting position so g_pos is always valid. */
    fen_to_board(UCI_STARTPOS, &g_pos);
    insert_past_move_entry(&g_pos);

    while (fgets(line, sizeof(line), stdin)) {
        /* Strip trailing CR / LF. */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';
        if (len == 0) continue;

        if      (strcmp(line, "uci")           == 0) handle_uci();
        else if (strcmp(line, "isready")        == 0) handle_isready();
        else if (strcmp(line, "ucinewgame")     == 0) handle_ucinewgame();
        else if (strncmp(line, "position", 8)   == 0) handle_position(line);
        else if (strncmp(line, "setoption", 9)  == 0) handle_setoption(line);
        /* Guard "go" so it doesn't match a hypothetical "goto" etc. */
        else if (strncmp(line, "go", 2) == 0
                 && (line[2] == ' ' || line[2] == '\0')) handle_go(line);
        else if (strcmp(line, "quit")           == 0) break;
        /* Unknown commands are silently ignored per the UCI specification. */
    }


}

