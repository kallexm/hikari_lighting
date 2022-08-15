#include "slab_event.h"
#include "events/slab_event_reset.h"
#include "events/slab_event_tick.h"
#include "events/slab_event_rgb.h"
#include "events/slab_event_hsv.h"

struct slab_event *slab_event_create(enum slab_event_id event_id, ...)
{
	struct slab_event *new_evt;
	va_list args;

	va_start(args, event_id);

	switch (event_id) {
	case SLAB_EVENT_RESET: {
		new_evt = slab_event_reset_create();
		break;
	}
	case SLAB_EVENT_TICK: {
		uint32_t time = va_arg(args, uint32_t);
		new_evt = slab_event_tick_create(time);
		break;
	}
	case SLAB_EVENT_RGB: {
		struct rgb_value val = va_arg(args, struct rgb_value);
		new_evt = slab_event_rgb_create(val);
		break;
	}
	case SLAB_EVENT_HSV: {
		struct hsv_value val = va_arg(args, struct hsv_value);
		new_evt = slab_event_hsv_create(val);
		break;
	}
	default:
		new_evt = NULL;
		goto clean_exit;
	}

	new_evt->id = event_id;
	new_evt->num_refs = 0;

clean_exit:
	va_end(args);
	return new_evt;
}

void slab_event_destroy(struct slab_event *evt)
{
	if (evt == NULL) {
		return;
	}

	switch (evt->id) {
	case SLAB_EVENT_RESET:
		slab_event_reset_destroy(evt);
		break;

	case SLAB_EVENT_TICK:
		slab_event_tick_destroy(evt);
		break;

	case SLAB_EVENT_RGB:
		slab_event_rgb_destroy(evt);
		break;

	case SLAB_EVENT_HSV:
		slab_event_hsv_destroy(evt);
		break;

	default:
		k_oops();
	}
}

void slab_event_acquire(struct slab_event *evt)
{
	if (evt == NULL) {
		return;
	}

	evt->num_refs += 1;
}

void slab_event_release(struct slab_event *evt)
{
	if (evt == NULL) {
		return;
	}

	evt->num_refs -= 1;

	if (evt->num_refs <= 0) {
		slab_event_destroy(evt);
	}
}
