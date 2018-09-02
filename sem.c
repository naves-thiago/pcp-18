#include <pthread.h>
#include <stdint.h>
#include "sem.h"

void sem_create(semaphore_t * sem, uint32_t count) {
	sem->count = count;
	pthread_mutex_init(&sem->mutex, NULL);
	pthread_cond_init(&sem->cond, NULL);
}

void sem_destroy(semaphore_t * sem) {
	pthread_mutex_destroy(&sem->mutex);
	pthread_cond_destroy(&sem->cond);
}

uint32_t sem_wait(semaphore_t * sem) {
	pthread_mutex_lock(&sem->mutex);
	while (!sem->count)
		pthread_cond_wait(&sem->cond, &sem->mutex);
	sem->count --;
	uint32_t c = sem->count;
	pthread_mutex_unlock(&sem->mutex);
	return c;
}

uint32_t sem_signal(semaphore_t * sem) {
	pthread_mutex_lock(&sem->mutex);
	uint32_t c = sem->count;
	sem->count ++;
	pthread_cond_signal(&sem->cond);
	pthread_mutex_unlock(&sem->mutex);
	return c;
}
