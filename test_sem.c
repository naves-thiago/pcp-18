#include "sem.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_t threads[4];
pthread_attr_t attr;
int id[] = {0, 1, 2, 3};
semaphore_t sem;

void * inc(void * p) {
	int id = *(int *)p;
	int count = 6 + id;
	usleep((rand() % 100) * 1000);
	printf("inc %d: start\n", id);

	for (int i=0; i<count; i++) {
		int c = sem_signal(&sem);
		printf("inc %d: signal - c = %d -> %d\n", id, c, c + 1);
		usleep((rand() % 100) * 1000);
	}
	printf("inc %d: done\n", id);
	return NULL;
}

void * dec(void * p) {
	int id = *(int *)p;
	int count = 3 + id;
	usleep((rand() % 100) * 1000);
	printf("dec %d: start\n", id);

	for (int i=0; i<count; i++) {
		int c = sem_wait(&sem);
		printf("dec %d: wait - c = %d -> %d\n", id, c+1, c);
		usleep((rand() % 100) * 1000);
	}
	printf("dec %d: done\n", id);
	return NULL;
}

int main() {
	srand(4);
	sem_create(&sem, 3);
	printf("Main: sem = 3\n");

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&threads[3], &attr, dec, &id[3]);
	pthread_create(&threads[0], &attr, inc, &id[0]);
	pthread_create(&threads[1], &attr, inc, &id[1]);
	pthread_create(&threads[2], &attr, dec, &id[2]);

	for (int i=0; i<4; i++)
		pthread_join(threads[i], NULL);

	sem_destroy(&sem);
	pthread_attr_destroy(&attr);
	pthread_exit(NULL);
	return 0;
}
