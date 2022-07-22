#include "slab_event.h"
#include "events/slab_event_rgb.h"

#include "slabs/slab_rgb2hsv.h"

struct slab *slab_rgb2hsv_create(void)
{
	struct slab *new_slab = k_malloc(sizeof(struct slab));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	return new_slab;
}

void slab_rgb2hsv_destroy(struct slab *slab)
{
	k_free(slab);
}

void slab_rgb2hsv_stim(struct slab *slab, struct slab_event *evt)
{
	switch (evt->id) {
	case SLAB_EVENT_RGB: {
		struct rgb_value rgb_val = slab_event_rgb_get_val(evt);

		struct hsv_value hsv_val = rgb2hsv(rgb_val);

		struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, hsv_val);
		slab_stim_childs(slab, hsv_evt);
		break;
	}
	default:
		slab_stim_childs(slab, evt);
	}
}
