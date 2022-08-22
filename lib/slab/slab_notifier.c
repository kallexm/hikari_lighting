#include "slab_event.h"

#include "slabs/slab_notifier.h"

struct slab *slab_notifier_create(slab_notifier_cb subscriber, void *context)
{
	struct slab_notifier *new_slab = k_malloc(sizeof(struct slab_notifier));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_slab->sub = subscriber;
	new_slab->ctx = context;

	return ((struct slab *)new_slab);
}

void slab_notifier_destroy(struct slab *slab)
{
	k_free(slab);
}

void slab_notifier_stim(struct slab *slab, struct slab_event *evt)
{
	struct slab_notifier *notifier = (struct slab_notifier *)slab;

	if (notifier->sub) {
		notifier->sub(evt, notifier->ctx);
	}

	slab_stim_childs(slab, evt);
}
