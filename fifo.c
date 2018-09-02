#include <stddef.h>
#include "fifo.h"

void fifo_init(FIFO * fifo, void ** buffer, uint32_t length) {
	fifo->write = 0;
	fifo->read = 0;
	fifo->length = length;
	fifo->buffer = buffer;
}

void fifo_clear(FIFO * fifo) {
	fifo->write = 0;
	fifo->read = 0;
}

uint32_t fifo_count(FIFO * fifo) {
	return (fifo->length - fifo->read + fifo->write) % fifo->length;
}

void fifo_push(FIFO * fifo, void * p) {
	fifo->buffer[fifo->write] = p;
	fifo->write = (fifo->write + 1) % fifo->length;
	if (fifo->write == fifo->read)
		fifo->read = (fifo->read + 1) % fifo->length;
}

bool fifo_peek(FIFO * fifo, void ** out, uint32_t n) {
	uint32_t c = fifo_count(fifo);
	if (n >= c)
		return false;

	uint32_t r = fifo->read;
	uint32_t l = fifo->length;
	*out = fifo->buffer[(r + n) % l];

	return true;
}

void fifo_remove(FIFO * fifo, uint32_t n) {
	if (n >= fifo_count(fifo)) {
		fifo->write = 0;
		fifo->read = 0;
		return;
	}

	fifo->read = (fifo->read + n) % fifo->length;
}
