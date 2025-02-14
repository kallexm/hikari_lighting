#ifndef SLAB_EVENT_HSV_H__
#define SLAB_EVENT_HSV_H__

#include <zephyr/kernel.h>

#include "slab_event.h"
#include "rgb_hsv.h"

struct slab_event_hsv {
	enum slab_event_id id;
	int num_refs;

	float h;
	float s;
	float v;
};

static inline struct slab_event *slab_event_hsv_create(struct hsv_value val)
{
	struct slab_event_hsv *new_evt = k_malloc(sizeof(struct slab_event_hsv));
	__ASSERT(new_evt != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_evt->h = val.h;
	new_evt->s = val.s;
	new_evt->v = val.v;

	return ((struct slab_event *)new_evt);
}

static inline void slab_event_hsv_destroy(struct slab_event *evt)
{
	k_free(evt);
}

static inline struct hsv_value slab_event_hsv_get_val(struct slab_event *evt)
{
	struct hsv_value val;
	struct slab_event_hsv *hsv_evt = (struct slab_event_hsv *)evt;
	val.h = hsv_evt->h;
	val.s = hsv_evt->s;
	val.v = hsv_evt->v;

	return val;
}

#endif /* SLAB_EVENT_HSV_H__ */
