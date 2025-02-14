#ifndef SLAB_EVENT_RGB_H__
#define SLAB_EVENT_RGB_H__

#include <zephyr/kernel.h>

#include "slab_event.h"
#include "rgb_hsv.h"

struct slab_event_rgb {
	enum slab_event_id id;
	int num_refs;

	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static inline struct slab_event *slab_event_rgb_create(struct rgb_value val)
{
	struct slab_event_rgb *new_evt = k_malloc(sizeof(struct slab_event_rgb));
	__ASSERT(new_evt != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_evt->r = val.r;
	new_evt->g = val.g;
	new_evt->b = val.b;

	return ((struct slab_event *)new_evt);
}

static inline void slab_event_rgb_destroy(struct slab_event *evt)
{
	k_free(evt);
}

static inline struct rgb_value slab_event_rgb_get_val(struct slab_event *evt)
{
	struct rgb_value val;
	struct slab_event_rgb *rgb_evt = (struct slab_event_rgb *)evt;

	val.r = rgb_evt->r;
	val.g = rgb_evt->g;
	val.b = rgb_evt->b;

	return val;
}

#endif /* SLAB_EVENT_RGB_H__ */
