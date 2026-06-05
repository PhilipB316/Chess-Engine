/**
* @file memory.h
* @brief Memory management for movefinding
* @author Philip Brand
* @date 2025-06-05
*
* Provides alternatives to malloc/free for memory allocation during movefinding.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "stddef.h"
#include "stdio.h"

#include "board.h"

#define POOL_SIZE 1000 // Adjust size as needed

extern Position_t *memory_pool[POOL_SIZE];
extern size_t pool_index;

/**
 * @brief Initializes the custom memory pool.
 */
void custom_memory_init(void);

/**
 * @brief De-initialises the custom memory pool, freeing all allocated memory.
 */
void custom_memory_deinit(void);

/**
 * @brief Allocates a Position_t from the custom memory pool.
 *
 * @return Pointer to the allocated Position_t, or NULL if the pool is exhausted.
 */
static inline Position_t* custom_alloc(void)
{
    if (SAFE && !web_build) {
        if (pool_index < POOL_SIZE) return memory_pool[pool_index++];
        fprintf(stderr, "ERROR: memory pool exhausted\n");
        return NULL;
    }
    return memory_pool[pool_index++];
}

/**
 * @brief Frees the last allocated Position_t from the custom memory pool.
 */
static inline void custom_free(void)
{
    if (SAFE) {
        if (pool_index) --pool_index;
    } else {
        --pool_index;
    }
}

/**
 * @brief Frees the last n allocated Position_t from the custom memory pool.
 */
void custom_free_n(uint16_t n);

/**
 * @brief Checks for memory leaks in the custom memory pool.
 *
 * Prints a message if there are any positions that were not freed.
 */
void check_memory_leak(void);

#endif // MEMORY_H
