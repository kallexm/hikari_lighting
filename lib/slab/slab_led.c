#include "slab_event.h"
#include "events/slab_event_rgb.h"

#include "slabs/slab_led.h"

struct slab *slab_led_create(void *led_buf, enum led_type type)
{
	struct slab_led *new_slab = k_malloc(sizeof(struct slab_led));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_slab->led = led_buf;
	new_slab->led_type = type;

	return ((struct slab *)new_slab);
}

void slab_led_destroy(struct slab *slab)
{
	k_free(slab);
}

static inline void write_led_buffer(struct slab_led *slab, struct rgb_value *val)
{
	switch (slab->led_type) {
	case LED_TYPE_RGB:
		slab->led[0] = val->g;
		slab->led[1] = val->r;
		slab->led[2] = val->b;
		break;

	case LED_TYPE_GRB:
		slab->led[0] = val->r;
		slab->led[1] = val->g;
		slab->led[2] = val->b;
		break;

	case LED_TYPE_RED:
		slab->led[0] = val->r;
		break;

	case LED_TYPE_GREEN:
		slab->led[0] = val->g;
		break;

	case LED_TYPE_BLUE:
		slab->led[0] = val->b;
		break;

	default:
		k_oops();
	}
}

void slab_led_stim(struct slab *slab, struct slab_event *evt)
{
	struct slab_led *led_slab = (struct slab_led *)slab;

	switch (evt->id) {
	case SLAB_EVENT_RGB:
		if (led_slab->led != NULL) {
			struct rgb_value rgb_val = slab_event_rgb_get_val(evt);
			write_led_buffer(led_slab, &rgb_val);
		}
		slab_stim_childs(slab, evt);
		break;

	default:
		slab_stim_childs(slab, evt);
		break;
	}
}
