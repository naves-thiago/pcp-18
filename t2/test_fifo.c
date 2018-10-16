#include "llfifo.h"
#include <stdlib.h>
#include <stdio.h>

#define test_assert(cond, ...) do {          \
	if (!(cond)) {                           \
		printf("Error line %d: ", __LINE__); \
		printf(__VA_ARGS__);                 \
		exit(1);                             \
	}                                        \
} while (0);

typedef struct {
	LLFifoItem item;
	int data;
} Item;

void test_init(void) {
	LLFifo f;
	llFifoInit(&f);
	test_assert(f.count == 0, "Count != 0\n");
	test_assert(f.head == NULL, "Head != NULL\n");
	test_assert(f.tail == NULL, "Tail != NULL\n");
}

void test_push(void) {
	LLFifo f;
	llFifoInit(&f);
	Item item, item2;
	llFifoPush(&f, (LLFifoItem *)&item);
	test_assert(f.count == 1, "Count != 1\n");
	test_assert(f.head == (LLFifoItem *)&item, "Head != item\n");
	test_assert(f.tail == (LLFifoItem *)&item, "Tail != item\n");
	llFifoPush(&f, (LLFifoItem *)&item2);
	test_assert(f.count == 2, "Count != 2\n");
	test_assert(f.head == (LLFifoItem *)&item, "Head != item\n");
	test_assert(f.tail == (LLFifoItem *)&item2, "Tail != item2\n");
}

void test_pop(void) {
	LLFifo f;
	llFifoInit(&f);
	Item item, item2;
	Item *p;
	llFifoPush(&f, (LLFifoItem *)&item);
	p = (Item *)llFifoPop(&f);
	test_assert(p == &item, "p != Item\n");
	test_assert(f.count == 0, "Count != 0\n");
	test_assert(f.head == NULL, "Head != NULL\n");
	test_assert(f.tail == NULL, "Tail != NULL\n");
	llFifoPush(&f, (LLFifoItem *)&item);
	llFifoPush(&f, (LLFifoItem *)&item2);
	p = (Item *)llFifoPop(&f);
	test_assert(p == &item, "p != Item\n");
	p = (Item *)llFifoPop(&f);
	test_assert(p == &item2, "p != Item2\n");
	test_assert(f.count == 0, "Count != 0\n");
	test_assert(f.head == NULL, "Head != NULL\n");
	test_assert(f.tail == NULL, "Tail != NULL\n");
	p = (Item *)llFifoPop(&f);
	test_assert(p == NULL, "p != NULL\n");
	test_assert(f.count == 0, "Count != 0\n");
	test_assert(f.head == NULL, "Head != NULL\n");
	test_assert(f.tail == NULL, "Tail != NULL\n");
}

int main() {
	test_init();
	test_push();
	test_pop();
	printf("OK\n");
	return 0;
}
