#include <stddef.h>
#include "fifo.h"

static inline uint32_t fifo_count_us(fifo_t * fifo);
static void fifo_remove_us(fifo_t * fifo, uint32_t n);
static bool fifo_peek_us(fifo_t * fifo, void ** out, uint32_t n);

void fifo_init(fifo_t * fifo, void ** buffer, uint32_t length) {
	fifo->write = 0;
	fifo->read = 0;
	fifo->length = length+1;
	fifo->buffer = buffer;
	pthread_mutex_init(&fifo->mutex, NULL);
}

void fifo_clear(fifo_t * fifo) {
	pthread_mutex_lock(&fifo->mutex);
	fifo->write = 0;
	fifo->read = 0;
	pthread_mutex_unlock(&fifo->mutex);
}

/* Not thread safe */
static inline uint32_t fifo_count_us(fifo_t * fifo) {
	return (fifo->length - fifo->read + fifo->write) % fifo->length;
}

uint32_t fifo_count(fifo_t * fifo) {
	pthread_mutex_lock(&fifo->mutex);
	uint32_t c = fifo_count_us(fifo);
	pthread_mutex_unlock(&fifo->mutex);
	return c;
}

void fifo_push(fifo_t * fifo, void * p) {
	pthread_mutex_lock(&fifo->mutex);
	fifo->buffer[fifo->write] = p;
	fifo->write = (fifo->write + 1) % fifo->length;
	if (fifo->write == fifo->read)
		fifo->read = (fifo->read + 1) % fifo->length;
	pthread_mutex_unlock(&fifo->mutex);
}

bool fifo_try_push(fifo_t * fifo, void * p) {
	pthread_mutex_lock(&fifo->mutex);
	uint32_t new_write = (fifo->write + 1) % fifo->length;
	if (new_write == fifo->read) {
		pthread_mutex_unlock(&fifo->mutex);
		return false;
	}

	fifo->buffer[fifo->write] = p;
	fifo->write = new_write;
	pthread_mutex_unlock(&fifo->mutex);
	return true;
}

static bool fifo_peek_us(fifo_t * fifo, void ** out, uint32_t n) {
	uint32_t c = fifo_count_us(fifo);
	if (n >= c)
		return false;

	uint32_t r = fifo->read;
	uint32_t l = fifo->length;
	*out = fifo->buffer[(r + n) % l];

	return true;
}

bool fifo_peek(fifo_t * fifo, void ** out, uint32_t n) {
	pthread_mutex_lock(&fifo->mutex);
	bool ret = fifo_peek_us(fifo, out, n);
	pthread_mutex_unlock(&fifo->mutex);
	return ret;
}

bool fifo_pop(fifo_t * fifo, void ** out) {
	pthread_mutex_lock(&fifo->mutex);
	bool ret = fifo_peek_us(fifo, out, 0);
	if (ret)
		fifo_remove_us(fifo, 1);
	pthread_mutex_unlock(&fifo->mutex);
	return ret;
}

static void fifo_remove_us(fifo_t * fifo, uint32_t n) {
	if (n >= fifo_count_us(fifo)) {
		fifo->write = 0;
		fifo->read = 0;
		return;
	}

	fifo->read = (fifo->read + n) % fifo->length;
}

void fifo_remove(fifo_t * fifo, uint32_t n) {
	pthread_mutex_lock(&fifo->mutex);
	fifo_remove_us(fifo, n);
	pthread_mutex_unlock(&fifo->mutex);
}

void fifo_destroy(fifo_t * fifo) {
	pthread_mutex_destroy(&fifo->mutex);
}
