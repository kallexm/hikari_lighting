#ifndef SLAB_TICKER_H__
#define SLAB_TICKER_H__

#include "zephyr/kernel.h"
#include "slab.h"

#define TICKER_WORKQUEUE_STACK_SIZE 2048
#define TICKER_WORKQUEUE_THREAD_PRIO 10

struct slab_ticker {
	sys_dlist_t childs;
	enum slab_type type;

	/* Specific data */
	k_timeout_t period;
	struct k_timer tick_timer;
	struct k_work tick_work;
};

struct slab *slab_ticker_create(k_timeout_t tick_period);

void slab_ticker_destroy(struct slab *slab);

void slab_ticker_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_TICKER_H__ */
