#ifndef SLAB_EVENT_H__
#define SLAB_EVENT_H__

#include <stdint.h>
#include <zephyr.h>

enum slab_event_id {
	SLAB_EVENT_RESET,
	SLAB_EVENT_TICK,
	SLAB_EVENT_RGB,
	SLAB_EVENT_HSV,
};

struct slab_event {
	enum slab_event_id id;
	int num_refs;
};

struct slab_event *slab_event_create(enum slab_event_id event_id, ...);
void slab_event_destroy(struct slab_event *evt);

void slab_event_acquire(struct slab_event *evt);
void slab_event_release(struct slab_event *evt);

#endif /* SLAB_EVENT_H__ */
