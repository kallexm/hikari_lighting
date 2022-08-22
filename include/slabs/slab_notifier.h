#ifndef SLAB_NOTIFIER_H__
#define SLAB_NOTIFIER_H__

#include "zephyr/kernel.h"
#include "slab.h"
#include "slab_event.h"

typedef void (*slab_notifier_cb)(struct slab_event *evt, void *ctx);

struct slab_notifier {
	sys_dlist_t childs;
	enum slab_type type;

	/* Specific data */
	slab_notifier_cb sub;
	void *ctx;
};

struct slab *slab_notifier_create(slab_notifier_cb subscriber, void *context);

void slab_notifier_destroy(struct slab *slab);

void slab_notifier_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_NOTIFIER_H__ */
