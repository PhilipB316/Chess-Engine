/**
* @file memory.h
* @brief Memory management for movefinding
* @author Philip Brand
* @date 2025-06-05
*
* Alternatives to malloc/free for memory allocation in movefinding.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include "board.h"

#define POOL_SIZE 1000 // Adjust size as needed

/**
 * @brief Initializes the custom memory pool.
 */
void custom_memory_init(void);

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

#endif // MEMORY_H
