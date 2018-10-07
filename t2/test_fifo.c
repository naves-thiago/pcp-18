#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static const size_t len = 10;
fifo_t fifo;

void test_clear(void) {
	fifo_clear(&fifo);
	assert(fifo.avaliable == 10);
	assert(fifo.used == 0);
	assert(fifo.write == 0);
	assert(fifo.read == 0);

	uint32_t data = 0xABCD;
	fifo_push(&fifo, &data);

	fifo_clear(&fifo);
	assert(fifo.avaliable == 10);
	assert(fifo.used == 0);
	assert(fifo.write == 0);
	assert(fifo.read == 0);
}

void test_push(void) {
	fifo_clear(&fifo);
	uint32_t data = 0;
	for (int i=0; i<len; i++)
		assert(fifo_push(&fifo, &data));

	assert(fifo_push(&fifo, &data) == false);
}

void test_pop(void) {
	fifo_clear(&fifo);
	uint32_t data = 0;
	assert(fifo_pop(&fifo, &data) == false);

	for (uint32_t i=0; i<len; i++)
		assert(fifo_push(&fifo, &i));

	for (uint32_t i=0; i<len; i++) {
		assert(fifo_pop(&fifo, &data));
		assert(data == i);
	}

	assert(fifo_pop(&fifo, &data) == false);
}

void test_push_pop(void) {
	fifo_clear(&fifo);
	uint32_t data;
	
	uint32_t first = len/2;
	for (uint32_t i=0; i<first; i++)
		assert(fifo_push(&fifo, &i));

	assert(fifo_pop(&fifo, &data));

	for (uint32_t i=0; i<len - first + 1; i++)
		assert(fifo_push(&fifo, &i));

	for (uint32_t i=1; i<first; i++) {
		assert(fifo_pop(&fifo, &data));
		assert(data == i);
	}

	for (uint32_t i=0; i<len - first + 1; i++) {
		assert(fifo_pop(&fifo, &data));
		assert(data == i);
	}

	assert(fifo_pop(&fifo, &data) == false);
}

int main() {
	uint8_t * data = malloc(len * sizeof(uint32_t));
	fifo_init(&fifo, sizeof(uint32_t), data, len * sizeof(uint32_t));
	test_clear();
	test_push();
	test_pop();
	test_push_pop();
	free(data);
	printf("OK\n");
	return 0;
}
