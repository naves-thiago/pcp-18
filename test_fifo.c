#include "fifo.h"
#include "sem.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define FIFO_LEN       10
#define ITERATIONS     100
#define THREADS_PUSH   3
#define THREADS_POP    4
#define THREAD_COUNT   ((THREADS_POP) + (THREADS_PUSH))

#define ERROR(...) do { printf(__VA_ARGS__); printf("Line: %d\n", __LINE__); } while (0);

pthread_t threads[THREAD_COUNT];
pthread_attr_t attr;
uint32_t id[THREAD_COUNT];
fifo_t fifo;
// Actual values used in the tests
int test_threads_push;
int test_threads_pop;
int test_threads_count;

void delay(void) {
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = (rand() % 100) * 100000;
	nanosleep(&t, NULL);
}

static void test_failed(void) {
	exit(1);
}

// Pushes numbers from 0 to ITERATIONS-1
void * push(void * p) {
	uint32_t id = *(uint32_t *) p;

	for (uint32_t i=0; i < ITERATIONS; i++) {
		delay();
		uint32_t tmp = (id << 16) | (i & UINT16_MAX);
		fifo_push(&fifo, (void *)tmp);
	}
	printf("Push done %u\n", id);
	return NULL;
}

void * pop(void * p) {
	uint32_t id = *(uint32_t *) p;
	printf("Start pop %u\n", id);

	// Separate counter to verify each push thread data by each pop thread
	semaphore_t counters[THREADS_PUSH];

	for (int i=0; i<test_threads_push; i++)
		sem_create(&counters[i], 0);

	for (volatile uint32_t i=0; i < ITERATIONS * test_threads_push; i++) {
		delay();
		volatile uint32_t data = UINT32_MAX;
		fifo_pop(&fifo, id, (void **)&data);
		uint32_t push_thread = data >> 16;
		uint32_t value = data & UINT16_MAX;
		uint32_t expected = sem_signal(&counters[push_thread]);
		if (value != expected) {
			ERROR("Pop (%u): expected %u, got value %u, push id %u\n", id, expected, value, push_thread);
			test_failed();
		}
	}

	for (int i=0; i<test_threads_push; i++)
		sem_destroy(&counters[i]);

	printf("Pop done %u\n", id);
	return NULL;
}

void test_single_threaded(void) {
	fifo_init(&fifo, 10, 3);
	fifo_clear(&fifo);

	printf("Test single threaded\n");
	for (uint32_t i=0; i<10; i++)
		fifo_push(&fifo, (void *)i);

	void * tmp;
	for (int i=0; i<3; i++) {
		fifo_pop(&fifo, i, &tmp);
		if (tmp != (void*)0) test_failed();
	}

	fifo_push(&fifo, (void *)10);

	for (uint32_t i=1; i<11; i++) {
		for (int k=0; k<3; k++) {
			fifo_pop(&fifo, k, &tmp);
			if (tmp != (void*)i) test_failed();
		}
	}
	fifo_destroy(&fifo);
	printf("Done.\n");
}

void test_single_push(void) {
	test_threads_pop = THREADS_POP;
	test_threads_push = 1;
	test_threads_count = 1 + THREADS_POP;

	fifo_init(&fifo, FIFO_LEN, test_threads_pop);
	fifo_clear(&fifo);

	printf("Test 1 push, %u pop\n", test_threads_pop);
	for (int i=0; i<test_threads_pop; i++)
		pthread_create(&threads[i], &attr, pop, &id[i]);

	pthread_create(&threads[THREADS_POP], &attr, push, &id[0]);

	for (int i=0; i<test_threads_pop; i++)
		pthread_join(threads[i], NULL);

	pthread_join(threads[test_threads_pop], NULL);

	fifo_destroy(&fifo);
	printf("Done.\n");
}

void test_single_pop(void) {
	test_threads_pop = 1;
	test_threads_push = THREADS_PUSH;
	test_threads_count = 1 + THREADS_PUSH;

	fifo_init(&fifo, FIFO_LEN, test_threads_pop);
	fifo_clear(&fifo);

	printf("Test 1 pop, %u push\n", test_threads_push);
	for (int i=0; i<test_threads_push; i++)
		pthread_create(&threads[i + test_threads_pop], &attr, push, &id[i]);

	pthread_create(&threads[0], &attr, pop, &id[0]);

	for (int i=0; i<test_threads_push; i++)
		pthread_join(threads[i + test_threads_pop], NULL);

	pthread_join(threads[0], NULL);

	fifo_destroy(&fifo);
	printf("Done.\n");
}

void test_multiple(void) {
	test_threads_pop = THREADS_POP;
	test_threads_push = THREADS_PUSH;
	test_threads_count = THREADS_POP + THREADS_PUSH;

	fifo_init(&fifo, FIFO_LEN, test_threads_pop);
	fifo_clear(&fifo);

	printf("Test %u pop, %u push\n", test_threads_pop, test_threads_push);
	for (int i=0; i<test_threads_push; i++)
		pthread_create(&threads[i + test_threads_pop], &attr, push, &id[i]);

	for (int i=0; i<test_threads_pop; i++)
		pthread_create(&threads[i], &attr, pop, &id[i]);

	for (int i=0; i<test_threads_count; i++)
		pthread_join(threads[i], NULL);

	fifo_destroy(&fifo);
	printf("Done.\n");
}

int main() {
	srand(time(NULL));

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int i=0; i<THREAD_COUNT; i++)
		id[i] = i;

	test_single_threaded();
	test_single_push();
	test_single_pop();
	test_multiple();


	pthread_attr_destroy(&attr);
	pthread_exit(NULL);
	return 0;
}
