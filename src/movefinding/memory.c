// memory.c

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "board.h"

static Position_t *memory_pool[POOL_SIZE];
static size_t pool_index = 0;

void custom_memory_init(void) 
{
    for (size_t i = 0; i < POOL_SIZE; i++) {
        memory_pool[i] = malloc(sizeof(Position_t));
    }
}

void custom_memory_deinit(void) 
{
    for (size_t i = 0; i < POOL_SIZE; i++) {
        free(memory_pool[i]);
    }
    pool_index = 0;
}

Position_t* custom_alloc(void) 
{
    if (SAFE && !web_build)
    {
        if (pool_index < POOL_SIZE) {
            return memory_pool[pool_index++];
        } else {
            printf("\n\n");
            printf(" %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
            printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
            printf(" %%%%      MEMORY POOL EXHAUSTED!        %%%% \n");
            printf(" %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
            printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
            printf("\n");
            return NULL; // Pool exhausted
        }
    } else {
        return memory_pool[pool_index++];
    }
}

void custom_free(void) 
{
    if (SAFE)
    {
        if (pool_index > 0) {
            pool_index--;
        }
    } else {
        pool_index--;
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
