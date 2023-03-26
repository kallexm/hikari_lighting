#include "slab_event.h"

#include "slabs/slab_delay.h"

static void reset_queue(struct slab_delay *slab)
{
	for (int i = 0; i < slab->length; i++) {
		slab->queue[i] = NULL;
	}
	slab->idx = 0;
}

static const struct slab_event *process_delay(struct slab_delay *slab, struct slab_event *evt_in)
{
	const struct slab_event *evt_out = slab->queue[slab->idx];

	slab->queue[slab->idx] = evt_in;

	slab->idx += 1;
	if (slab->idx >= slab->length) {
		slab->idx = 0;
	}

	return (struct slab_event *)evt_out;
}

struct slab *slab_delay_create(uint32_t delay_periods)
{
	struct slab_delay *new_slab = k_malloc(sizeof(struct slab_delay));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_slab->queue = k_malloc(sizeof(struct slab_event *)*delay_periods);
	__ASSERT(new_slab->queue != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");
	
	new_slab->length = delay_periods;

	reset_queue(new_slab);

	return ((struct slab *)new_slab);
}

void slab_delay_destroy(struct slab *slab)
{
	struct slab_delay *delay = (struct slab_delay *)slab;

	k_free(delay->queue);

	k_free(delay);
}

void slab_delay_stim(struct slab *slab, struct slab_event *evt)
{
	struct slab_delay *delay_slab = (struct slab_delay *)slab;

	switch (evt->id) {
	case SLAB_EVENT_RESET:
		reset_queue(delay_slab);
		slab_stim_childs(slab, evt);
		break;

	case SLAB_EVENT_RGB:
	case SLAB_EVENT_HSV:
	case SLAB_EVENT_TICK: {
		struct slab_event *evt_to_send = (struct slab_event *)process_delay(delay_slab, evt);
		if (evt_to_send != NULL) {
			slab_stim_childs(slab, evt_to_send);
		}
		break;
	}
	default:
		slab_stim_childs(slab, evt);
	}
}
