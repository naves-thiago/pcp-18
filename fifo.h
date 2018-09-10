#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "sem.h"

/**
 * @defgroup fifo FIFO
 * @brief Thread-safe pointer FIFO for multiple consumers.
 *        All consumers must pop a pointer for the space become available.
 *        Blocks on push if full.
 *        For each consumer, block on pop if empty.
 * @{
 */

/**
 * @brief pointer FIFO instance
 */
typedef struct {
	void ** buffer;            /**< Data area                                */
	uint32_t write;            /**< Next write position                      */
	uint32_t * read;           /**< Oldest data position for each consumer   */
	uint32_t length;           /**< FIFO length                              */
	uint32_t consumers;        /**< Number of consumers                      */
	uint32_t * pend_pop;       /**< Pending pops for each pointer            */
	pthread_cond_t * cond_pop; /**< Conditions to block on pop               */
	semaphore_t sem_push;      /**< Block on push semaphore                  */
	pthread_mutex_t mutex;
} fifo_t;

/**
 * @brief Initializes the FIFO and sets the data area
 *
 * @param[in] fifo The FIFO instance
 * @param[in] length Length of FIFO
 * @param[in] consumers Number of consumers
 */
void fifo_init(fifo_t * fifo, uint32_t length, uint32_t consumers);

/**
 * @brief Removes all data from the FIFO
 *
 * @param[in] fifo The FIFO instance
 */
void fifo_clear(fifo_t * fifo);

/**
 * @brief Adds a pointer to the FIFO. If the FIFO is full, the oldest pointer is replaced
 *
 * @param[in] fifo The FIFO instance
 * @param[in] p The pointer to add
 */
void fifo_push(fifo_t * fifo, void * p);

/**
 * @brief Copy the oldest pointer to @p out and removes it from the FIFO
 *
 * @param[in] fifo The FIFO instance
 * @param[in] consumer ID of the consumer (0 to consumers - 1)
 * @param[out] out Copy destination
 */
void fifo_pop(fifo_t * fifo, uint32_t consumer, void ** out);

/**
 * @brief Releases any resources held by the FIFO
 *
 * @param[in] fifo The FIFO instance
 */
void fifo_destroy(fifo_t * fifo);

/** @} */
