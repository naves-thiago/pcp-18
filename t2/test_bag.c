#include "bag.h"
#include <stdlib.h>
#include <stdio.h>

#define test_assert(cond, ...) do {          \
	if (!(cond)) {                           \
		printf("Error line %d: ", __LINE__); \
		printf(__VA_ARGS__);                 \
		exit(1);                             \
	}                                        \
} while (0);

void test_init(void) {
	bag_t b;
	bag_init(&b, 0);
	test_assert(bag_count(&b) == 0, "Count != 0\n");
	bag_free(&b);
}

void test_pop(void) {
	bag_t b;
	bool res;
	interval_t i, i2;

	bag_init(&b, 0);
	test_assert(bag_count(&b) == 0, "Count != 0\n");
	res = bag_pop(&b, &i2);
	test_assert(res == false, "Res != false\n");
	i.start = 3;
	i.end = 4;
	i.area = 5;
	bag_push(&b, i);
	test_assert(bag_count(&b) == 1, "Count != 1\n");
	res = bag_pop(&b, &i2);
	test_assert(res, "Res != true\n");
	test_assert(i2.start == i.start, "i2.start != i.start\n");
	test_assert(i2.end == i.end, "i2.end != i.end\n");
	test_assert(i2.area == i.area, "i2.area != i.area\n");
	test_assert(bag_count(&b) == 0, "Count != 0\n");
	res = bag_pop(&b, &i2);
	test_assert(res == false, "Res != false\n");
	bag_free(&b);

	bag_init(&b, 1);
	test_assert(bag_count(&b) == 0, "Count != 0\n");
	i.start = 3;
	i.end = 4;
	i.area = 5;
	bag_push(&b, i);
	test_assert(bag_count(&b) == 1, "Count != 1\n");
	res = bag_pop(&b, &i2);
	test_assert(res, "Res != true\n");
	test_assert(i2.start == i.start, "i2.start != i.start\n");
	test_assert(i2.end == i.end, "i2.end != i.end\n");
	test_assert(i2.area == i.area, "i2.area != i.area\n");
	test_assert(bag_count(&b) == 0, "Count != 0\n");
	res = bag_pop(&b, &i2);
	test_assert(res == false, "Res != false\n");
	bag_free(&b);

	bag_init(&b, 0);
	i.start = 3;
	i.end = 4;
	i.area = 1;
	bag_push(&b, i);
	i.start = 5;
	i.end = 6;
	i.area = 2;
	bag_push(&b, i);
	test_assert(bag_count(&b) == 2, "Count != 2\n");
	res = bag_pop(&b, &i2);
	test_assert(res, "Res != true\n");
	test_assert(i2.start == 3, "i2.start != 3\n");
	test_assert(i2.end == 4, "i2.end != 4\n");
	test_assert(i2.area == 1, "i2.end != 1\n");
	test_assert(bag_count(&b) == 1, "Count != 1\n");
	res = bag_pop(&b, &i2);
	test_assert(res, "Res != true\n");
	test_assert(i2.start == 5, "i2.start != 3\n");
	test_assert(i2.end == 6, "i2.end != 4\n");
	test_assert(i2.area == 2, "i2.end != 2\n");
	test_assert(bag_count(&b) == 0, "Count != 1\n");
	res = bag_pop(&b, &i2);
	test_assert(res == false, "Res != false\n");
	bag_free(&b);

	bag_init(&b, 10);
	i.start = 3;
	i.end = 4;
	i.area = 1;
	bag_push(&b, i);
	i.start = 5;
	i.end = 6;
	i.area = 2;
	bag_push(&b, i);
	test_assert(bag_count(&b) == 2, "Count != 2\n");
	res = bag_pop(&b, &i2);
	test_assert(res, "Res != true\n");
	test_assert(i2.start == 3, "i2.start != 3\n");
	test_assert(i2.end == 4, "i2.end != 4\n");
	test_assert(i2.area == 1, "i2.end != 1\n");
	test_assert(bag_count(&b) == 1, "Count != 1\n");
	res = bag_pop(&b, &i2);
	test_assert(res, "Res != true\n");
	test_assert(i2.start == 5, "i2.start != 3\n");
	test_assert(i2.end == 6, "i2.end != 4\n");
	test_assert(i2.area == 2, "i2.end != 2\n");
	test_assert(bag_count(&b) == 0, "Count != 1\n");
	res = bag_pop(&b, &i2);
	test_assert(res == false, "Res != false\n");
	bag_free(&b);
}

int main() {
	test_init();
	test_pop();
	printf("OK\n");
	return 0;
}

