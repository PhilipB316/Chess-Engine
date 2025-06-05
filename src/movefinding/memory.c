// memory.c

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

static Position_t *memory_pool[POOL_SIZE];
static size_t pool_index = 0;

void custom_memory_init(void) 
{
    for (size_t i = 0; i < POOL_SIZE; i++) {
        memory_pool[i] = malloc(sizeof(Position_t));
    }
}

Position_t* custom_alloc(void) 
{
    if (SAFE) 
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
