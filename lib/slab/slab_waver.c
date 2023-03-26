#include "slab_event.h"
#include "events/slab_event_tick.h"

#include "slabs/slab_waver.h"

#include "wave_func.h"


struct slab *slab_waver_create(struct slab_waver_config *config)
{
	struct slab_waver *new_slab = k_malloc(sizeof(struct slab_waver));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");
	
	new_slab->data.h = config->hue;
	new_slab->data.s = config->sat;
	new_slab->data.v = 0;

	new_slab->gen = wave_func_create(&(config->val));

	return ((struct slab *)new_slab);
}

void slab_waver_destroy(struct slab *slab)
{
	struct slab_waver *waver_slab = (struct slab_waver *)slab;
	wave_func_destroy(waver_slab->gen);

	k_free(slab);
}

void slab_waver_stim(struct slab *slab, struct slab_event *evt)
{
	struct slab_waver *waver_slab = (struct slab_waver *)slab;

	switch (evt->id) {
	case SLAB_EVENT_RESET:
		wave_func_reset(waver_slab->gen, NULL);

		slab_stim_childs(slab, evt);
		break;

	case SLAB_EVENT_TICK: {
		uint32_t time = slab_event_tick_get_time(evt);
		waver_slab->data.v = wave_func_process(waver_slab->gen, time);

		struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, waver_slab->data);
		slab_event_acquire(hsv_evt);

		slab_stim_childs(slab, hsv_evt);

		slab_stim_childs(slab, evt);
		break;
	}

	default:
		slab_stim_childs(slab, evt);
		break;
	}
}
