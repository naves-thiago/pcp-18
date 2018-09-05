#include "fifo.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define FIFO_LEN       30
#define ITERATIONS     100
#define THREADS_PUSH   3
#define THREADS_POP    4
#define THREAD_COUNT   ((THREADS_POP) + (THREADS_PUSH))

#define ERROR(...) do { printf(__VA_ARGS__); printf("Line: %d\n", __LINE__); } while (0);

pthread_t threads[THREAD_COUNT];
pthread_attr_t attr;
uint32_t id[THREAD_COUNT];
pthread_mutex_t mutex[THREADS_PUSH];
uint32_t push_counter[THREADS_PUSH];
bool push_done[THREADS_PUSH];
fifo_t fifo;
void * fifo_mem[FIFO_LEN + 1];

static inline void test_failed(void) {
	exit(1);
}

// Pushes numbers from 1 to ITERATIONS without overrides
void * try_push(void * p) {
	uint32_t id = *(uint32_t *) p;

	uint32_t i = 1;
	while (i <= ITERATIONS) {
		usleep((rand() % 100) * 100);
		uint32_t tmp = (id << 16) | (i & UINT16_MAX);
		i += fifo_try_push(&fifo, (void *)tmp);
	}
	return NULL;
}

// Pops numbers and verifies if they are in sequence per push thread until
// ITERATIONS numbers have been popped for each thread
void * pop_seq(void * p) {
	(void) p;

	while (true) {
		usleep((rand() % 100) * 100);
		for (int i=0; i<THREADS_PUSH; i++)
			if (push_counter[i] < ITERATIONS)
				goto not_done;

		return NULL; // all push counters == ITERACTIONS -> test done
		uint32_t tmp;
not_done:
		if (!fifo_pop(&fifo, (void **)&tmp))
			continue;
		
		uint32_t id = tmp >> 16;
		uint32_t val = tmp & UINT16_MAX;
		if (id >= THREADS_PUSH) {
			ERROR("Invalid ID: %u. Value: %u\n", id, val);
			test_failed();
		}

		if (val > ITERATIONS) {
			ERROR("Invalid Value: %u. ID: %u\n", val, id);
			test_failed();
		}

		pthread_mutex_lock(&mutex[id]);
		if (push_counter[id] != val - 1) {
			ERROR("Non sequential value: %u. Old: %u. ID: %u\n", val, push_counter[id], id);
			pthread_mutex_unlock(&mutex[id]);
			test_failed();
		}

		push_counter[id] = val;
		pthread_mutex_unlock(&mutex[id]);
	}
	return NULL;
}

// Pushes numbers from 1 to ITERATIONS possibly overriding
void * push(void * p) {
	uint32_t id = *(uint32_t *) p;

	uint32_t i = 1;
	while (i <= ITERATIONS) {
		usleep((rand() % 100) * 100);
		uint32_t tmp = (id << 16) | (i & UINT16_MAX);
		fifo_push(&fifo, (void *)tmp);
		i++;
	}
	push_done[id] = true;
	return NULL;
}

// Pops numbers and verifies if they are larger then the previus per push thread until
// push threads are done and the FIFO empty
void * pop(void * p) {
	(void) p;

	while (true) {
		usleep((rand() % 100) * 100);
		for (int i=0; i<THREADS_PUSH; i++)
			if (!push_done[i])
				goto not_done;

		if (fifo_count(&fifo) == 0)
			return NULL; // all push threads done and FIFO empty -> test done
		uint32_t tmp;
not_done:
		if (!fifo_pop(&fifo, (void **)&tmp))
			continue;
		
		uint32_t id = tmp >> 16;
		uint32_t val = tmp & UINT16_MAX;
		if (id >= THREADS_PUSH) {
			ERROR("Invalid ID: %u. Value: %u\n", id, val);
			test_failed();
		}

		if (val > ITERATIONS) {
			ERROR("Invalid Value: %u. ID: %u\n", val, id);
			test_failed();
		}

		pthread_mutex_lock(&mutex[id]);
		if (push_counter[id] >= val) {
			ERROR("Non sequential value: %u. Old: %u. ID: %u\n", val, push_counter[id], id);
			pthread_mutex_unlock(&mutex[id]);
			test_failed();
		}

		push_counter[id] = val;
		pthread_mutex_unlock(&mutex[id]);
	}
	return NULL;
}

void test_count(void) {
	printf("Test count (single threaded)\n");
	fifo_clear(&fifo);
	if (fifo_count(&fifo)) {
		ERROR("Count failed\n");
		test_failed();
	}

	for (int i=0; i<FIFO_LEN; i++) {
		fifo_push(&fifo, NULL);
		uint32_t tmp = fifo_count(&fifo);
		if (tmp != i+1) {
			ERROR("Count failed. Expected %u, got %u\n", i+1, tmp);
			test_failed();
		}
	}

	fifo_push(&fifo, NULL);
	uint32_t tmp = fifo_count(&fifo);
	if (tmp != FIFO_LEN) {
		ERROR("Count failed. Expected %u, got %u\n", FIFO_LEN, tmp);
		test_failed();
	}
	printf("Done.\n");
}

void test_peek(void) {
	printf("Test peek (single threaded)\n");
	uint32_t tmp;
	fifo_clear(&fifo);
	if (fifo_peek(&fifo, (void **)&tmp, 0)) {
		ERROR("Peek failed\n");
		test_failed();
	}

	if (fifo_peek(&fifo, (void **)&tmp, 1)) {
		ERROR("Peek failed\n");
		test_failed();
	}

#if FIFO_LEN > 1
	if (fifo_peek(&fifo, (void **)&tmp, FIFO_LEN)) {
		ERROR("Peek failed\n");
		test_failed();
	}
#endif

#if FIFO_LEN > 2
	if (fifo_peek(&fifo, (void **)&tmp, FIFO_LEN -1)) {
		ERROR("Peek failed\n");
		test_failed();
	}
#endif

#if FIFO_LEN > 3
	if (fifo_peek(&fifo, (void **)&tmp, FIFO_LEN -2)) {
		ERROR("Peek failed\n");
		test_failed();
	}
#endif

	tmp = 1;
	fifo_push(&fifo, (void *)tmp);

	if (!fifo_peek(&fifo, (void **)&tmp, 0)) {
		ERROR("Peek failed\n");
		test_failed();
	}

	if (tmp != 1) {
		ERROR("Peek failed. Expected 1, got %u\n", tmp);
		test_failed();
	}

	if (fifo_peek(&fifo, (void **)&tmp, 1)) {
		ERROR("Peek failed\n");
		test_failed();
	}

#if FIFO_LEN > 1
	if (fifo_peek(&fifo, (void **)&tmp, FIFO_LEN)) {
		ERROR("Peek failed\n");
		test_failed();
	}
#endif

#if FIFO_LEN > 2
	if (fifo_peek(&fifo, (void **)&tmp, FIFO_LEN -1)) {
		ERROR("Peek failed\n");
		test_failed();
	}
#endif

#if FIFO_LEN > 3
	if (fifo_peek(&fifo, (void **)&tmp, FIFO_LEN -2)) {
		ERROR("Peek failed\n");
		test_failed();
	}
#endif

	fifo_clear(&fifo);
	for (uint32_t i=0; i<FIFO_LEN+1; i++)
		fifo_push(&fifo, (void *)i);

	for (uint32_t i=0; i<FIFO_LEN; i++) {
		if (!fifo_peek(&fifo, (void **)&tmp, i)) {
			ERROR("Peek failed\n");
			test_failed();
		}
		if (tmp != i+1) {
			ERROR("Peek failed. Expected %u, got %u\n", i+1, tmp);
			test_failed();
		}
	}
	printf("Done.\n");
}

void test_try_push(void) {
	fifo_clear(&fifo);
	for (int i=0; i<THREADS_PUSH; i++) {
		push_counter[i] = 0;
		push_done[i] = false;
		pthread_mutex_init(&mutex[i], NULL);
	}

	for (int i=0; i<THREAD_COUNT; i++)
		id[i] = i;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	printf("Test try_push and pop_seq\n");
	printf("%u threads pushing, %u threads popping\n", THREADS_PUSH, THREADS_POP);
	for (int i=0; i<THREADS_PUSH; i++) {
		push_counter[i] = 0;
		pthread_mutex_init(&mutex[i], NULL);
	}

	for (int i=0; i<THREADS_PUSH; i++)
		pthread_create(&threads[i], &attr, try_push, &id[i]);

	for (int i=0; i<THREADS_POP; i++)
		pthread_create(&threads[i + THREADS_PUSH], &attr, pop_seq, &id[i + THREADS_PUSH]);

	for (int i=0; i<THREAD_COUNT; i++)
		pthread_join(threads[i], NULL);

	for (int i=0; i<THREADS_PUSH; i++)
		pthread_mutex_destroy(&mutex[i]);

	printf("Done.\n");
}

void test_push(void) {
	printf("Test push and pop...\n");
	printf("%u threads pushing, %u threads popping\n", THREADS_PUSH, THREADS_POP);
	fifo_clear(&fifo);
	for (int i=0; i<THREADS_PUSH; i++) {
		push_counter[i] = 0;
		push_done[i] = false;
		pthread_mutex_init(&mutex[i], NULL);
	}

	for (int i=0; i<THREADS_PUSH; i++)
		pthread_create(&threads[i], &attr, push, &id[i]);

	for (int i=0; i<THREADS_POP; i++)
		pthread_create(&threads[i + THREADS_PUSH], &attr, pop, &id[i + THREADS_PUSH]);

	for (int i=0; i<THREAD_COUNT; i++)
		pthread_join(threads[i], NULL);

	for (int i=0; i<THREADS_PUSH; i++)
		pthread_mutex_destroy(&mutex[i]);

	printf("Done.\n");
}

int main() {
	srand(time(NULL));
	fifo_init(&fifo, fifo_mem, FIFO_LEN);

	test_count();
	test_peek();
	test_try_push();
	test_push();

	pthread_attr_destroy(&attr);
	pthread_exit(NULL);
	return 0;
}
