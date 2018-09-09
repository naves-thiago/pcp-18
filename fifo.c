#include <stdlib.h>
#include <stddef.h>
#include "fifo.h"

#if 0
#include <stdio.h>
#define log(...) printf(__VA_ARGS__)
#else
#define log(...)
#endif

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
	log("push %u\n", (uint32_t)p);
	log("push W sem_push\n");
	sem_wait(&fifo->sem_push);
	log("push W mutex\n");
	pthread_mutex_lock(&fifo->mutex);
	log("push G mutex\n");
	fifo->buffer[fifo->write] = p;
	fifo->pend_pop[fifo->write] = fifo->consumers;
	uint32_t w = fifo->write;
//	pthread_cond_broadcast(&fifo->cond_pop[fifo->write]);
	fifo->write = (fifo->write + 1) % fifo->length;
	log("push BC %u\n", w);
	pthread_cond_broadcast(&fifo->cond_pop[w]);
	pthread_mutex_unlock(&fifo->mutex);
	log("push R mutex\n");
}

void fifo_pop(fifo_t * fifo, uint32_t consumer, void ** out) {
	log("pop W mutex\n");
	pthread_mutex_lock(&fifo->mutex);
	log("pop G mutex\n");
	uint32_t r = fifo->read[consumer];
//	if (r != fifo->write)
//		fifo->read[consumer] = (r + 1) % fifo->length;
	
	log("pop W cond %u\n", r);
	while (!fifo->pend_pop[r])
		pthread_cond_wait(&fifo->cond_pop[r], &fifo->mutex);
	log("pop G cond\n");

	fifo->read[consumer] = (r + 1) % fifo->length;
	fifo->pend_pop[r] --;
	*out = fifo->buffer[r];
	if (!fifo->pend_pop[r]) {
		sem_signal(&fifo->sem_push);
		log("pop S sem_push\n");
	}
	pthread_mutex_unlock(&fifo->mutex);
	log("pop R mutex\n");
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
