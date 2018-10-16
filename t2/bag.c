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
