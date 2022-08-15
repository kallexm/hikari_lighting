#ifndef SLAB_EVENT_TICK_H__
#define SLAB_EVENT_TICK_H__

#include <zephyr.h>

#include "slab_event.h"

struct slab_event_tick {
	enum slab_event_id id;
	int num_refs;

	uint32_t time;
};

static inline struct slab_event *slab_event_tick_create(uint32_t time)
{
	struct slab_event_tick *new_evt = k_malloc(sizeof(struct slab_event_tick));
	__ASSERT(new_evt != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_evt->time = time;

	return ((struct slab_event *)new_evt);
}

static inline void slab_event_tick_destroy(struct slab_event *evt)
{
	k_free(evt);
}

static inline uint32_t slab_event_tick_get_time(struct slab_event *evt)
{
	uint32_t time;
	struct slab_event_tick *tick_evt = (struct slab_event_tick *)evt;

	time = tick_evt->time;

	return time;
}

#endif /* SLAB_EVENT_TICK_H__ */
