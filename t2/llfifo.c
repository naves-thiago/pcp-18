/* Linked list FIFO lib */
#include "llfifo.h"
#include <string.h>
#include <string.h>
#include <assert.h>

void llFifoInit(LLFifo * fifo) {
  fifo->head = NULL;
  fifo->tail = NULL;
  fifo->count = 0;
}

LLFifoItem * llFifoPop(LLFifo * fifo) {
  assert(fifo != NULL);

  if (fifo->count == 0)
    return NULL;

  LLFifoItem * ret = fifo->head;
  if (fifo->head == fifo->tail) {
    fifo->head = NULL;
    fifo->tail = NULL;
  }
  else {
    fifo->head = fifo->head->next;
    fifo->head->prev = NULL;
  }

  fifo->count --;
  ret->status = LL_FIFO_READ;
  return ret;
}

void llFifoPush(LLFifo * fifo, LLFifoItem * item) {
  assert(fifo != NULL);
  assert(item != NULL);

  if (fifo->tail == NULL) {
    fifo->tail = item;
    fifo->head = item;
    item->next = NULL;
    item->prev = NULL;
  }
  else {
    fifo->tail->next = item;
    item->prev = fifo->tail;
    item->next = NULL;
    fifo->tail = item;
  }
  item->status = LL_FIFO_QUEUED;
  fifo->count++;
}

bool llFifoDelete(LLFifo * fifo, LLFifoItem * item) {
  assert(fifo != NULL);
  assert(item != NULL);

  if (item->status == LL_FIFO_DELETED)
    return true;

  if (item->status == LL_FIFO_READ)
    return false;

  if (item->status == LL_FIFO_QUEUED) {
    item->status = LL_FIFO_DELETED;

    if (fifo->head == item)
      fifo->head = item->next;

    if (fifo->tail == item)
      fifo->tail = item->prev;

    if (item->prev)
      item->prev->next = item->next;

    if (item->next)
      item->next->prev = item->prev;

    fifo->count--;
    return true;
  }

  /* Unexpected / invalid status code. should never get here */
  assert(false);
  //return false;
}

uint32_t llFifoCount(LLFifo * fifo) {
	assert(fifo);
	return fifo->count;
}
