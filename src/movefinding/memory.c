// memory.c

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "board.h"

Position_t *memory_pool[POOL_SIZE];
size_t pool_index = 0;

void custom_memory_init(void) 
{
    for (size_t i = 0; i < POOL_SIZE; i++) {
        memory_pool[i] = calloc(1, sizeof(Position_t));
    }
}

void custom_memory_deinit(void) 
{
    for (size_t i = 0; i < POOL_SIZE; i++) {
        free(memory_pool[i]);
    }
    pool_index = 0;
}

void custom_free_n(uint16_t n) {
    if (SAFE) {
        pool_index = (pool_index >= n) ? pool_index - n : 0;
    } else {
        pool_index -= n;
    }
}

void check_memory_leak(void) 
{
    if (pool_index > 0) {
        printf("\n\n-leak-detected-%zu-positions-leaked---\n", pool_index);
    } else {
        printf("---no-leaks-detected---\n");
    }
}
