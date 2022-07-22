#include "slab_event.h"
#include "events/slab_event_hsv.h"

#include "slabs/slab_hsv2rgb.h"

struct slab *slab_hsv2rgb_create(void)
{
	struct slab *new_slab = k_malloc(sizeof(struct slab));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	return new_slab;
}

void slab_hsv2rgb_destroy(struct slab *slab)
{
	k_free(slab);
}

void slab_hsv2rgb_stim(struct slab *slab, struct slab_event *evt)
{
	switch (evt->id) {
	case SLAB_EVENT_HSV: {
		struct hsv_value hsv_val = slab_event_hsv_get_val(evt);

		struct rgb_value rgb_val = hsv2rgb(hsv_val);

		struct slab_event *rgb_evt = slab_event_create(SLAB_EVENT_RGB, rgb_val);
		slab_stim_childs(slab, rgb_evt);
		break;
	}
	default:
		slab_stim_childs(slab, evt);
	}
}
