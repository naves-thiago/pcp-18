#pragma once
#include "llfifo.h"
#include <stdbool.h>

typedef struct {
	double start;
	double end;
} interval_t;

typedef struct {
	LLFifo fifo;
	LLFifo pool;
} bag_t;

void bag_init(bag_t * b, uint32_t initial_capacity);
void bag_push(bag_t * b, interval_t i);
bool bag_pop(bag_t * b, interval_t * i);
void bag_free(bag_t * b);
uint32_t bag_count(bag_t * b);
