#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/**
 * @defgroup fifo FIFO
 * @brief Thread-safe pointer circular FIFO.
 *        Push with a full FIFO overrides the oldest pointer.
 * @{
 */

/**
 * @brief pointer FIFO instance
 */
typedef struct {
	void ** buffer;         /**< Data area (must be length + 1 in size) */
	uint32_t  write;        /**< Next write position                    */
	uint32_t  read;         /**< Oldest data position                   */
	uint32_t  length;       /**< FIFO length                            */
	pthread_mutex_t mutex;
} fifo_t;

/**
 * @brief Initializes the FIFO and sets the data area
 *
 * @param[in] fifo The FIFO instance
 * @param[in] buffer The memory area to store the data
 * @param[in] length Length of @p buffer
 */
void fifo_init(fifo_t * fifo, void ** buffer, uint32_t length);

/**
 * @brief Removes all data from the FIFO
 *
 * @param[in] fifo The FIFO instance
 */
void fifo_clear(fifo_t * fifo);

/**
 * @brief Counts the number of queued pointers
 *
 * @param[in] fifo The FIFO instance
 * @return The number of pointers in the FIFO
 */
uint32_t fifo_count(fifo_t * fifo);

/**
 * @brief Adds a pointer to the FIFO. If the FIFO is full, the oldest pointer is replaced
 *
 * @param[in] fifo The FIFO instance
 * @param[in] p The pointer to add
 */
void fifo_push(fifo_t * fifo, void * p);

/**
 * @brief Adds a pointer to the FIFO if there is available space.
 *
 * @param[in] fifo The FIFO instance
 * @param[in] p The pointer to add
 *
 * @return True successfull, false if no space was available
 */
bool fifo_try_push(fifo_t * fifo, void * p);

/**
 * @brief Copy the n-th oldest pointer to @p out
 *
 * @param[in] fifo The FIFO instance
 * @param[out] out Copy destination
 * @param[in] n Index (from oldest to newest) of the pointer to read
 *
 * @return true on success, false if the index is empty or out of bounds.
 *
 * @sa fifo_remove() fifo_pop()
 */
bool fifo_peek(fifo_t * fifo, void ** out, uint32_t count);

/**
 * @brief Copy the oldest pointer to @p out and removes it from the FIFO
 *
 * @param[in] fifo The FIFO instance
 * @param[out] out Copy destination
 *
 * @return true on success, false if the FIFO is empty.
 *
 * @sa fifo_remove() fifo_peek()
 */
bool fifo_pop(fifo_t * fifo, void ** out);

/**
 * @brief Remove the @p n oldest pointers from the FIFO
 *
 * @param[in] fifo The FIFO instance
 * @param[in] n Amount of pointers to remove
 *
 * @sa fifo_peek() fifo_pop()
 */
void fifo_remove(fifo_t * fifo, uint32_t n);

/**
 * @brief Releases any resources held by the FIFO
 *
 * @param[in] fifo The FIFO instance
 */
void fifo_destroy(fifo_t * fifo);

/** @} */
