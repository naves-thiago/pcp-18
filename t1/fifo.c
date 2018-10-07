#include <stdlib.h>
#include <stddef.h>
#include "fifo.h"

void fifo_init(fifo_t * fifo, uint32_t length, uint32_t consumers) {
	fifo->buffer = (void **)malloc((length + 1) * sizeof(void*));
	fifo->read = (uint32_t *)calloc(consumers, sizeof(uint32_t));
	fifo->sem_pop = (semaphore_t *)malloc((length + 1) * sizeof(semaphore_t));
	for (uint32_t i=0; i<length + 1; i++)
		sem_create(&fifo->sem_pop[i], 0);
	fifo->write = 0;
	fifo->length = length + 1;
	fifo->consumers = consumers;
	sem_create(&fifo->sem_push, length);
	pthread_mutex_init(&fifo->mutex, NULL);
}

void fifo_clear(fifo_t * fifo) {
	pthread_mutex_lock(&fifo->mutex);
	fifo->write = 0;
	
	for (uint32_t i=0; i<fifo->consumers; i++)
		fifo->read[i] = 0;

	for (uint32_t i=0; i<fifo->length; i++)
		sem_set(&fifo->sem_pop[i], 0);

	sem_set(&fifo->sem_push, fifo->length-1);
	pthread_mutex_unlock(&fifo->mutex);
}

void fifo_push(fifo_t * fifo, void * p) {
	sem_wait(&fifo->sem_push);
	pthread_mutex_lock(&fifo->mutex);
	fifo->buffer[fifo->write] = p;
	uint32_t w = fifo->write;
	fifo->write = (fifo->write + 1) % fifo->length;
	pthread_mutex_unlock(&fifo->mutex);
	sem_set(&fifo->sem_pop[w], fifo->consumers);
}

void fifo_pop(fifo_t * fifo, uint32_t consumer, void ** out) {
	uint32_t r = fifo->read[consumer];
	uint32_t remaining = sem_wait(&fifo->sem_pop[r]);

	fifo->read[consumer] = (r + 1) % fifo->length;

	*out = fifo->buffer[r];
	if (!remaining)
		sem_signal(&fifo->sem_push);
}

void fifo_destroy(fifo_t * fifo) {
	pthread_mutex_destroy(&fifo->mutex);
	for (uint32_t i=0; i<fifo->length; i++)
		sem_destroy(&fifo->sem_pop[i]);

	sem_destroy(&fifo->sem_push);
	free(fifo->buffer);
	free(fifo->read);
	free(fifo->sem_pop);
}
