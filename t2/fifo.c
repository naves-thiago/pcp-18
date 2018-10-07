#include "fifo.h"
#include <string.h>

void fifo_init(fifo_t * fifo, size_t element_size, uint8_t * data, size_t size) {
	fifo->element_size = element_size;
	fifo->data = data;
	fifo->size = size - (size % element_size);
	fifo->write = 0;
	fifo->read = 0;
	fifo->avaliable = size / element_size;
	fifo->used = 0;
}

bool fifo_push(fifo_t * fifo, void * data) {
	if (!fifo)
		return false;

	if (!fifo->avaliable)
		return false;

	fifo->avaliable --;
	memcpy(&fifo->data[fifo->write], data, fifo->element_size);
	fifo->write = (fifo->write + fifo->element_size) % fifo->size;
	fifo->used ++;
	return true;
}

bool fifo_pop(fifo_t * fifo, void * data) {
	if (!fifo)
		return false;

	if (!fifo->used)
		return false;

	fifo->used --;
	memcpy(data, &fifo->data[fifo->read], fifo->element_size);
	fifo->read = (fifo->read + fifo->element_size) % fifo->size;
	fifo->avaliable ++;
	return true;
}

void fifo_clear(fifo_t * fifo) {
	if (!fifo)
		return;

	fifo->write = 0;
	fifo->read = 0;
	fifo->avaliable += fifo->used;
	fifo->used = 0;
}
