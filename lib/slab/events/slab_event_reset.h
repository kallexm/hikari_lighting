#ifndef SLAB_EVENT_RESET_H__
#define SLAB_EVENT_RESET_H__

#include <zephyr.h>

#include "slab_event.h"

static inline struct slab_event *slab_event_reset_create(void)
{
	struct slab_event *new_evt = k_malloc(sizeof(struct slab_event));
	__ASSERT(new_evt != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	return new_evt;
}

static inline void slab_event_reset_destroy(struct slab_event *evt)
{
	k_free(evt);
}

#endif /* SLAB_EVENT_RESET_H__ */
