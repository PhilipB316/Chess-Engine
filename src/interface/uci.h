/**
 * @file uci.h
 * @brief UCI (Universal Chess Interface) protocol handler.
 *
 * Supported commands: uci, isready, setoption (stub), ucinewgame,
 *                     position, go, quit.
 *
 * No dependency on ui.c, movedisplay.c, or their headers.
 *
 * @author Philip Brand
 * @date 2026-06-09
 */

#ifndef UCI_H
#define UCI_H

/**
 * @brief Runs the UCI command loop.
 *
 * Reads commands from stdin and writes responses to stdout until
 * "quit" is received or EOF. The caller is responsible for
 * initialising engine subsystems (custom_memory_init, move_finder_init,
 * zobrist_key_init, hash_table_init) before calling this function, and
 * for tearing them down afterwards.
 */
void uci_loop(void);

#endif // UCI_H

