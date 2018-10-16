#include "bag.h"
#include "llfifo.h"
#include <stdlib.h>

/**
 * @brief This struct stores an integration interval in the bag (FIFO)
 */
typedef struct {
	LLFifoItem item;
	interval_t interval;
} bag_item_t;


void bag_init(bag_t * b, uint32_t initial_capacity) {
	llFifoInit(&b->fifo);
	llFifoInit(&b->pool);

	for (uint32_t i=0; i<initial_capacity; i++) {
		bag_item_t * item = (bag_item_t *)malloc(sizeof(bag_item_t));
		llFifoPush(&b->pool, (LLFifoItem *)item);
	}
}

void bag_push(bag_t * b, interval_t i) {
	bag_item_t * item = (bag_item_t *)llFifoPop(&b->pool);
	if (!item)
		item = (bag_item_t *)malloc(sizeof(bag_item_t));

	item->interval = i;
	llFifoPush(&b->fifo, (LLFifoItem *)item);
}

bool bag_pop(bag_t * b, interval_t * i) {
	LLFifoItem * item = llFifoPop(&b->fifo);
	if (!item)
		return false;

	*i = ((bag_item_t *)item)->interval;
	llFifoPush(&b->pool, item);
	return true;
}

void bag_free(bag_t * b) {
	bag_item_t * item;
	
	while ((item = (bag_item_t *)llFifoPop(&b->fifo)))
		free(item);

	while ((item = (bag_item_t *)llFifoPop(&b->pool)))
		free(item);
}

uint32_t bag_count(bag_t * b) {
	return llFifoCount(&b->fifo);
}
