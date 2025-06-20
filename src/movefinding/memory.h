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

#include "board.h"

#define POOL_SIZE 1000 // Adjust size as needed

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
Position_t* custom_alloc(void);

/**
 * @brief Frees the last allocated Position_t from the custom memory pool.
 */
void custom_free(void);

/**
 * @brief Checks for memory leaks in the custom memory pool.
 *
 * Prints a message if there are any positions that were not freed.
 */
void check_memory_leak(void);

#endif // MEMORY_H
