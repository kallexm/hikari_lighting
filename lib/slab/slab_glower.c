#include <zephyr/random/random.h>

#include "slab_event.h"
#include "events/slab_event_tick.h"

#include "slabs/slab_glower.h"

#include "glow_func.h"


static inline float random_value()
{
	return (float)(sys_rand32_get() & 0x03FF) / 1024.0f;
}


struct slab *slab_glower_create(struct slab_glower_config *config)
{
	struct slab_glower *new_slab = k_malloc(sizeof(struct slab_glower));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");
	
	new_slab->data.h = config->hue;
	new_slab->data.s = config->sat;
	new_slab->data.v = 0;

	new_slab->gen = glow_func_create(&(config->val));

	return ((struct slab *)new_slab);
}

void slab_glower_destroy(struct slab *slab)
{
	struct slab_glower *glower_slab = (struct slab_glower *)slab;
	glow_func_destroy(glower_slab->gen);

	k_free(slab);
}

void slab_glower_stim(struct slab *slab, struct slab_event *evt)
{
	struct slab_glower *glower_slab = (struct slab_glower *)slab;

	switch (evt->id) {
	case SLAB_EVENT_RESET:
		glow_func_reset(glower_slab->gen, NULL);

		slab_stim_childs(slab, evt);
		break;

	case SLAB_EVENT_TICK: {
		uint32_t time = slab_event_tick_get_time(evt);
		float rand_val = random_value();
		glower_slab->data.v = glow_func_process(glower_slab->gen, time, rand_val);

		struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, glower_slab->data);
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
