#ifndef SLAB_DELAY_H__
#define SLAB_DELAY_H__

#include <stdint.h>

#include "slab.h"

struct slab_delay {
	sys_dlist_t childs;
	enum slab_type type;

	const struct slab_event** queue;
	uint32_t length;
	uint32_t idx;
};

struct slab *slab_delay_create(uint32_t delay_periods);

void slab_delay_destroy(struct slab *slab);

void slab_delay_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_DELAY_H__ */
