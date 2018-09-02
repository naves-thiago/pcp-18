#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup fifo FIFO
 * @brief Non thread-safe pointer circular FIFO.
 *         Push with a full FIFO overrides the oldest pointer.
 * @{
 */

/**
 * @brief pointer FIFO instance
 */
typedef struct {
	void ** buffer;       /**< Data area (must be length + 1 in size) */
	uint32_t  write;      /**< Next write position                    */
	uint32_t  read;       /**< Oldest data position                   */
	uint32_t  length;     /**< FIFO length                            */
} FIFO;

/**
 * @brief Initializes the FIFO and sets the data area
 *
 * @param[in] fifo The FIFO instance
 * @param[in] buffer The memory area to store the data
 * @param[in] length Length of @p buffer
 */
void fifo_init(FIFO * fifo, void ** buffer, uint32_t length);

/**
 * @brief Removes all data from the FIFO
 *
 * @param[in] fifo The FIFO instance
 */
void fifo_clear(FIFO * fifo);

/**
 * @brief Counts the number of queued pointers
 *
 * @param[in] fifo The FIFO instance
 * @return The number of pointers in the FIFO
 */
uint32_t fifo_count(FIFO * fifo);

/**
 * @brief Adds a pointer to the FIFO. If the FIFO is full, the oldest pointer is replaced
 *
 * @param[in] fifo The FIFO instance
 * @param[in] p The pointer to add
 */
void fifo_push(FIFO * fifo, void * p);

/**
 * @brief Copy the n-th oldest pointer to @p out
 *
 * @param[in] fifo The FIFO instance
 * @param[out] out Copy destination
 * @param[in] n Index (from oldest to newest) of the pointer to read
 *
 * @return true on success, false if the index is empty of out of bounds.
 *
 * @sa fifo_pop()
 */
bool fifo_peek(FIFO * fifo, void ** out, uint32_t count);

/**
 * @brief Move the oldest pointers in the FIFO (up to @p count) to @p out
 * @note If @p out is NULL, the pointers will be discarded instead
 *
 * @param[in] fifo The FIFO instance
 * @param[out] out Copy destination or NULL
 * @param[in] count Maximum amount of pointers to move
 *
 * @return The number of pointers actually moved
 *
 * @sa fifo_peek()
 */
uint32_t fifo_pop(FIFO * fifo, void ** out, uint32_t count);

/** @} */
