/**
 * @file web_main.c
 * @author Philip Brand
 * @brief Main file for building the web interface version of the chess engine
 * @date 2025-11-16
 */

#include <stdint.h>

#include "./movefinding/movefinder.h"
#include "./movefinding/memory.h"
#include "./search/hash_tables.h"
#include "./interface/uci.h"

#define new "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

int main(void)
{
    custom_memory_init();
    move_finder_init();
    zobrist_key_init();
    hash_table_init();
    uci_loop();
    custom_memory_deinit();
    hash_table_free();
}
