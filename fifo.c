#include <stdlib.h>
#include <stddef.h>
#include "fifo.h"

void fifo_init(fifo_t * fifo, uint32_t length, uint32_t consumers) {
	fifo->buffer = (void **)malloc((length + 1) * sizeof(void*));
	fifo->read = (uint32_t *)calloc(consumers, sizeof(uint32_t));
	fifo->pend_pop = (uint32_t *)calloc((length + 1), sizeof(uint32_t));
	fifo->cond_pop = (pthread_cond_t *)malloc((length + 1) * sizeof(pthread_cond_t));
	for (uint32_t i=0; i<length + 1; i++)
		pthread_cond_init(&fifo->cond_pop[i], NULL);
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
		fifo->pend_pop[i] = 0;

	sem_set(&fifo->sem_push, fifo->length-1);
	pthread_mutex_unlock(&fifo->mutex);
}

void fifo_push(fifo_t * fifo, void * p) {
	sem_wait(&fifo->sem_push);
	pthread_mutex_lock(&fifo->mutex);
	fifo->buffer[fifo->write] = p;
	fifo->pend_pop[fifo->write] = fifo->consumers;
	uint32_t w = fifo->write;
	fifo->write = (fifo->write + 1) % fifo->length;
	pthread_cond_broadcast(&fifo->cond_pop[w]);
	pthread_mutex_unlock(&fifo->mutex);
}

void fifo_pop(fifo_t * fifo, uint32_t consumer, void ** out) {
	pthread_mutex_lock(&fifo->mutex);
	uint32_t r = fifo->read[consumer];

	while (!fifo->pend_pop[r])
		pthread_cond_wait(&fifo->cond_pop[r], &fifo->mutex);

	fifo->read[consumer] = (r + 1) % fifo->length;
	fifo->pend_pop[r] --;
	*out = fifo->buffer[r];
	if (!fifo->pend_pop[r])
		sem_signal(&fifo->sem_push);

	pthread_mutex_unlock(&fifo->mutex);
}

void fifo_destroy(fifo_t * fifo) {
	pthread_mutex_destroy(&fifo->mutex);
	for (uint32_t i=0; i<fifo->length; i++)
		pthread_cond_destroy(&fifo->cond_pop[i]);

	sem_destroy(&fifo->sem_push);
	free(fifo->buffer);
	free(fifo->read);
	free(fifo->cond_pop);
	free(fifo->pend_pop);
}
