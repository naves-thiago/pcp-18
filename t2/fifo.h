#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t * data;
	size_t element_size;
	size_t size;
	size_t write;
	size_t read;
	size_t avaliable;
	size_t used;
} fifo_t;

/**
 * @brief Initializes the FIFO.
 */
void fifo_init(fifo_t * fifo, size_t element_size, uint8_t * data, size_t size);

/**
 * @brief Copies data into the FIFO. Fails if full.
 * @return true on success, false otherwise.
 */
bool fifo_push(fifo_t * fifo, void * data);

/**
 * @brief Copies data from the FIFO into @p data. Fails if empty.
 * @return true on success, false otherwise.
 */
bool fifo_pop(fifo_t * fifo, void * data);

/**
 * @brief Removes all data from the FIFO.
 */
void fifo_clear(fifo_t * fifo);
