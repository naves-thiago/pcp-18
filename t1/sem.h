#pragma once

#include <stdint.h>
#include <pthread.h>

/**
 * @defgroup sem Semaphore
 * @brief Counting semaphore based on pthreads
 * @{
 */

typedef struct {
	uint32_t count;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} semaphore_t;

/**
 * @brief Creates a semaphore and set the initial count
 */
void sem_create(semaphore_t * sem, uint32_t count);

/**
 * @brief Destroys the semaphore
 */
void sem_destroy(semaphore_t * sem);

/**
 * @brief Decrements the semaphore counter, blocking the thread if the count is 0
 * @return The counter value AFTER decrementint it
 */
uint32_t sem_wait(semaphore_t * sem);

/**
 * @brief Increments the semaphore counter
 * @return The counter value BEFORE incrementing it
 */
uint32_t sem_signal(semaphore_t * sem);

/**
 * @brief Sets the semaphore counter. If count > 0, also nofities the waiting threads
 */
void sem_set(semaphore_t * sem, uint32_t count);

/** @} */
